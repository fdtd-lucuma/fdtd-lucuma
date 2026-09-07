// == pipeline/4_fdtd_setup.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pipeline/4_fdtd_setup.hpp"

#include <algorithm>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <utility>

#include "auxiliaries/funcs.hpp"
#include "fdtd/fw_sim.hpp"

namespace lucuma::julia {

namespace {

// 64-bit FNV-1a over the raw bytes of the calibration inputs.
struct Fnv {
	std::uint64_t h = 1469598103934665603ull;
	void bytes(const void* p, std::size_t n) {
		const auto* b = static_cast<const unsigned char*>(p);
		for (std::size_t i = 0; i < n; ++i) {
			h ^= b[i];
			h *= 1099511628211ull;
		}
	}
	void u(std::uint64_t v) { bytes(&v, sizeof v); }
	void d(double v) { bytes(&v, sizeof v); }
	void s(std::string_view v) {
		bytes(v.data(), v.size());
		u(v.size());
	}
	void c(std::complex<double> v) {
		d(v.real());
		d(v.imag());
	}
	template <class M>
	void mat(const M& m) {
		u(static_cast<std::uint64_t>(m.rows()));
		u(static_cast<std::uint64_t>(m.cols()));
		for (Eigen::Index i = 0; i < m.size(); ++i) c(m.data()[i]);
	}
	std::string hex() const {
		char out[17];
		std::snprintf(out, sizeof out, "%016llx",
		              static_cast<unsigned long long>(h));
		return out;
	}
};

void HashMode(Fnv& f, const VectorModeProfile& m) {
	f.c(m.neff);
	f.c(m.beta);
	f.mat(m.Ex);
	f.mat(m.Ey);
	f.mat(m.Ez);
	f.mat(m.Hx);
	f.mat(m.Hy);
	f.mat(m.Hz);
}

}  // namespace

std::array<int, 3> WholeRegionSizes(const Params& params) {
	const double dx = TomlNum(params, "domain", "dx");
	const double dy = TomlNum(params, "domain", "dy");
	const double dz = TomlNum(params, "domain", "dz");
	return {static_cast<int>(julia_round(TomlNum(params, "domain", "Lx") / dx)),
	        static_cast<int>(julia_round(TomlNum(params, "domain", "Ly") / dy)),
	        static_cast<int>(julia_round(TomlNum(params, "domain", "Lz") / dz))};
}

SimRegion BuildSimRegion(const Params& params) {
	const double Lx = TomlNum(params, "domain", "Lx");
	const double Ly = TomlNum(params, "domain", "Ly");
	const double Lz = TomlNum(params, "domain", "Lz");
	const double dx = TomlNum(params, "domain", "dx");
	const double dy = TomlNum(params, "domain", "dy");
	const double dz = TomlNum(params, "domain", "dz");
	const double pxy = TomlNum(params, "pml", "xy");
	const double pz = TomlNum(params, "pml", "z");

	return {ValuesToIndx(pxy, Lx - pxy, dx), ValuesToIndx(pxy, Ly - pxy, dy),
	        ValuesToIndx(pz, Lz - pz, dz)};
}

BBox BuildBoundingBox(const std::string& name, const Params& device,
                      const Params& params, bool with_direction) {
	static constexpr std::array<std::pair<const char*, const char*>, 3> DIMS{
	    {{"x", "dx"}, {"y", "dy"}, {"z", "dz"}}};

	BBox b;
	b.name = name;
	for (int d = 0; d < 3; ++d) {
		const auto& [coord, step] = DIMS[d];
		auto arr = device[name][coord].as_array();
		const auto rng = ValuesToIndx((*arr)[0].value<double>().value(),
		                              (*arr)[1].value<double>().value(),
		                              TomlNum(params, "domain", step));
		b.r[d] = {rng[0], rng[1]};
	}
	if (with_direction)
		b.direction = TomlStr(device, name, "direction");
	return b;
}

FDTDParams FDTDSetUp(const std::filesystem::path& params_path,
                     const std::filesystem::path& device_path,
                     const std::filesystem::path& region_path,
                     const std::vector<EquivalentModalSource>& modal_sources,
                     const std::vector<std::string>& source_area_names,
                     const std::vector<std::string>& waveguide_names,
                     std::map<std::string, ModeMonitor> monitors,
                     const std::vector<std::string>& monitor_names) {
	const Params params = LoadParams(params_path);
	const Params device = LoadParams(device_path);

	FDTDParams fp;
	fp.sources = CreateSources(modal_sources);
	fp.monitors = std::move(monitors);
	fp.whole_region_sizes = WholeRegionSizes(params);
	fp.sim_region = BuildSimRegion(params);

	for (const auto& n : source_area_names)
		fp.source_areas.push_back(BuildBoundingBox(n, device, params, true));
	for (const auto& n : waveguide_names)
		fp.waveguides_areas.push_back(BuildBoundingBox(n, device, params, true));
	for (const auto& n : monitor_names)
		fp.monitors_areas.push_back(BuildBoundingBox(n, device, params, false));

	fp.opt_region = BuildBoundingBox("P", device, params, false);

	auto dim = device["P"]["dim"].as_array();
	fp.nxP = static_cast<int>((*dim)[0].value<int64_t>().value());
	fp.nyP = static_cast<int>((*dim)[1].value<int64_t>().value());
	fp.nzP = static_cast<int>((*dim)[2].value<int64_t>().value());
	fp.opt_region_values = LoadP(fp.nxP, fp.nyP, fp.nzP, region_path);

	fp.eps0 = TomlNum(params, "material", "eps0");
	fp.eps_fg = TomlNum(params, "material", "eps_fg");
	fp.eps_bg = TomlNum(params, "material", "eps_bg");
	fp.sigma_m = TomlNum(params, "material", "sigma_m");
	fp.sigma_fg = TomlNum(params, "material", "sigma_fg");
	fp.sigma_bg = TomlNum(params, "material", "sigma_bg");
	fp.mu = TomlNum(params, "material", "mu0");

	fp.pml_m = static_cast<int>(TomlNum(params, "pml", "m"));
	fp.pml_xy = TomlNum(params, "pml", "xy");
	fp.pml_z = TomlNum(params, "pml", "z");
	fp.sigma_max_xy = TomlNum(params, "simulation", "sigma_max_xy");
	fp.sigma_max_z = TomlNum(params, "simulation", "sigma_max_z");

	fp.dx = TomlNum(params, "domain", "dx");
	fp.dy = TomlNum(params, "domain", "dy");
	fp.dz = TomlNum(params, "domain", "dz");
	fp.dt = TomlNum(params, "simulation", "dt");
	fp.t = TomlInt(params, "simulation", "t");
	fp.freqs = TomlF64Array(params, "simulation", "freqs");

	// Coefficients are filled by the backend's init(), not here.
	return fp;
}

std::string SourceNormalizationSignature(
    const FDTDParams& params, const std::vector<std::string>& monitor_names) {
	Fnv f;
	f.u(SOURCE_NORMALIZATION_SCHEMA_VERSION);

	for (int n : params.whole_region_sizes) f.u(static_cast<std::uint64_t>(n));
	for (const auto& r : params.sim_region) {
		f.u(static_cast<std::uint64_t>(r[0]));
		f.u(static_cast<std::uint64_t>(r[1]));
	}
	for (const auto& b : params.waveguides_areas) {
		f.s(b.name);
		for (const auto& r : b.r) {
			f.u(static_cast<std::uint64_t>(r[0]));
			f.u(static_cast<std::uint64_t>(r[1]));
		}
	}
	f.d(params.eps0);
	f.d(params.eps_fg);
	f.d(params.eps_bg);
	f.d(params.sigma_m);
	f.d(params.sigma_fg);
	f.d(params.sigma_bg);
	f.d(params.mu);
	f.d(params.dx);
	f.d(params.dy);
	f.d(params.dz);
	f.u(static_cast<std::uint64_t>(params.t));
	f.d(params.dt);
	f.u(static_cast<std::uint64_t>(params.pml_m));
	f.d(params.pml_xy);
	f.d(params.pml_z);
	f.d(params.sigma_max_xy);
	f.d(params.sigma_max_z);
	for (double fr : params.freqs) f.d(fr);

	for (const auto& s : params.sources) {
		f.u(static_cast<std::uint64_t>(s.axis));
		f.u(static_cast<std::uint64_t>(s.electric_fixed_index));
		f.u(static_cast<std::uint64_t>(s.magnetic_fixed_index));
		f.d(s.J0);
		f.d(s.time_origin);
		f.d(s.inv_delta_normal);
		f.u(static_cast<std::uint64_t>(s.size1));
		f.u(static_cast<std::uint64_t>(s.size2));
		f.u(static_cast<std::uint64_t>(s.nfreqs));
		f.u(static_cast<std::uint64_t>(s.r1_lo));
		f.u(static_cast<std::uint64_t>(s.r2_lo));
		for (double w : s.omegas) f.d(w);
		for (const auto* v : {&s.Jx, &s.Jy, &s.Jz, &s.Mx, &s.My, &s.Mz})
			for (const auto& z : *v) f.c(z);
	}

	std::vector<std::string> names = monitor_names;
	std::sort(names.begin(), names.end());
	for (const auto& name : names) {
		auto it = params.monitors.find(name);
		if (it == params.monitors.end()) continue;
		const ModeMonitor& m = it->second;
		f.s(name);
		f.u(static_cast<std::uint64_t>(m.axis));
		f.u(static_cast<std::uint64_t>(m.polarity));
		f.u(static_cast<std::uint64_t>(m.fixed_index));
		f.u(static_cast<std::uint64_t>(m.range1.lo));
		f.u(static_cast<std::uint64_t>(m.range1.hi));
		f.u(static_cast<std::uint64_t>(m.range2.lo));
		f.u(static_cast<std::uint64_t>(m.range2.hi));
		f.d(m.delta1);
		f.d(m.delta2);
		f.d(m.delta_normal);
		for (const auto& [freq, mode] : m.modes_by_freq) {
			f.d(freq);
			HashMode(f, mode);
		}
	}
	return f.hex();
}

bool SourceNormalizationCacheIsCurrent(const std::filesystem::path& norm_path,
                                       const std::string& expected_signature) {
	if (!std::filesystem::is_regular_file(norm_path)) return false;
	toml::table data;
	try {
		data = toml::parse_file(norm_path.string());
	} catch (...) {
		return false;
	}
	const int64_t ver = data["schema_version"].value_or<int64_t>(0);
	const std::string sig = data["signature"].value_or(std::string{});
	return ver == SOURCE_NORMALIZATION_SCHEMA_VERSION &&
	       sig == expected_signature;
}

SourceNormalization LoadSourceNormalization(
    const std::filesystem::path& norm_path) {
	const toml::table data = toml::parse_file(norm_path.string());
	SourceNormalization sn;

	if (auto* a = data["monitor_names"].as_array())
		for (auto&& e : *a) sn.monitor_names.push_back(e.value_or(std::string{}));
	if (auto* a = data["freqs"].as_array())
		for (auto&& e : *a) sn.freqs.push_back(e.value_or(0.0));
	if (auto* a = data["total_power"].as_array())
		for (auto&& e : *a) sn.power.push_back(e.value_or(0.0));

	if (auto* p = data["powers"].as_table())
		for (auto&& [k, v] : *p) {
			std::vector<double> vv;
			if (auto* a = v.as_array())
				for (auto&& e : *a) vv.push_back(e.value_or(0.0));
			sn.power_by_monitor[std::string(k.str())] = std::move(vv);
		}

	auto* re = data["overlaps_real"].as_table();
	auto* im = data["overlaps_imag"].as_table();
	if (re && im)
		for (auto&& [k, v] : *re) {
			const std::string key(k.str());
			auto* ra = v.as_array();
			auto* ia = (*im)[key].as_array();
			std::vector<cdouble> vv;
			if (ra && ia)
				for (std::size_t i = 0; i < ra->size(); ++i)
					vv.emplace_back((*ra)[i].value_or(0.0),
					                (*ia)[i].value_or(0.0));
			sn.overlap_by_monitor[key] = std::move(vv);
		}
	return sn;
}

void SaveSourceNormalization(const SourceNormalization& sn,
                             const std::filesystem::path& norm_path,
                             const std::string& signature) {
	if (norm_path.has_parent_path())
		std::filesystem::create_directories(norm_path.parent_path());

	toml::table t;
	t.insert("schema_version",
	         static_cast<int64_t>(SOURCE_NORMALIZATION_SCHEMA_VERSION));
	t.insert("signature", signature);

	toml::array mn;
	for (const auto& s : sn.monitor_names) mn.push_back(s);
	t.insert("monitor_names", mn);

	toml::array fr;
	for (double v : sn.freqs) fr.push_back(v);
	t.insert("freqs", fr);

	toml::array tp;
	for (double v : sn.power) tp.push_back(v);
	t.insert("total_power", tp);

	toml::table powers;
	for (const auto& [k, v] : sn.power_by_monitor) {
		toml::array a;
		for (double x : v) a.push_back(x);
		powers.insert(k, a);
	}
	t.insert("powers", powers);

	toml::table oreal, oimag;
	for (const auto& [k, v] : sn.overlap_by_monitor) {
		toml::array ar, ai;
		for (const auto& z : v) {
			ar.push_back(z.real());
			ai.push_back(z.imag());
		}
		oreal.insert(k, ar);
		oimag.insert(k, ai);
	}
	t.insert("overlaps_real", oreal);
	t.insert("overlaps_imag", oimag);

	std::ofstream f(norm_path);
	f << t;
}

}  // namespace lucuma::julia
