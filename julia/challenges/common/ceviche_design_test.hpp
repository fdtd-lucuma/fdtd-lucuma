// == challenges/ceviche_designs_test/<name>/<name>_design_test.jl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Deviation from the .jl: excitation uses ConstantEnvelope, not
// RampedContinuousWave (the windowed DFT at the end is steady-state either way).

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "auxiliaries/array.hpp"
#include "auxiliaries/funcs.hpp"
#include "auxiliaries/modes.hpp"
#include "auxiliaries/sources.hpp"
#include "fdtd/backends/instantiator.hpp"
#include "fdtd/fw_sim.hpp"
#include "fdtd/params.hpp"
#include "fdtd/utils.hpp"
#include "pipeline/3_wg_source.hpp"
#include "pipeline/4_fdtd_setup.hpp"
#include "utils/precision.hpp"

namespace lucuma::julia {

namespace ceviche_design_test {

namespace fs = std::filesystem;

struct MetricRow {
	int frequency_index;
	double wavelength_nm;
	double frequency_hz;
	int input_port;
	int output_port;
	double s_real;
	double s_imag;
	double transmission;
};

inline std::string num(double x) {
	std::ostringstream os;
	os << std::setprecision(17) << x;
	return os.str();
}

template <class F>
inline void write_plane_csv(const fs::path& path, int n1, int n2, F&& value) {
	std::ofstream out(path, std::ios::binary | std::ios::trunc);
	for (int i = 1; i <= n1; ++i) {
		for (int j = 1; j <= n2; ++j) {
			if (j > 1) out << ',';
			out << num(value(i, j));
		}
		out << '\n';
	}
}

inline int ModeIndexOf(const Params& device, const std::string& name) {
	return static_cast<int>(
	    device[name]["mode_index"].value<std::int64_t>().value_or(1));
}

template <class T>
int Run(const fs::path& input_dir, const fs::path& out_base, Backend backend) {
	const fs::path params_path = input_dir / "params.toml";
	const fs::path device_path = input_dir / "device.toml";
	const fs::path out_dir = out_base / "results" / "all_results";
	fs::create_directories(out_dir);

	const Params params = LoadParams(params_path);
	const Params device = LoadParams(device_path);

	const std::array<int, 3> dims = WholeRegionSizes(params);
	const int Nx = dims[0], Ny = dims[1];

	const double dx = TomlNum(params, "domain", "dx");
	const double dy = TomlNum(params, "domain", "dy");
	const double dz = TomlNum(params, "domain", "dz");

	const double eps0 = TomlNum(params, "material", "eps0");
	const double mu0 = TomlNum(params, "material", "mu0");
	const double c0 = TomlNum(params, "material", "c0");
	const double eps_fg = TomlNum(params, "material", "eps_fg");
	const double eps_bg = TomlNum(params, "material", "eps_bg");

	const double design_value = TomlNum(params, "simulation", "design_value");
	const double courant = TomlNum(params, "simulation", "courant");
	const long ramp_cycles = static_cast<long>(TomlNum(params, "simulation", "ramp_cycles"));
	const int nmodes = static_cast<int>(TomlNum(params, "simulation", "nmodes"));
	const double source_amplitude = TomlNum(params, "simulation", "source_amplitude");
	const long steps = static_cast<long>(TomlNum(params, "simulation", "steps"));
	const long dft_cycles = static_cast<long>(TomlNum(params, "simulation", "dft_cycles"));
	(void)ramp_cycles;

	const std::vector<double> freqs = TomlF64Array(params, "simulation", "freqs");

	const BBox design_box = BuildBoundingBox("P", device, params, false);
	const int nxP = static_cast<int>(design_box.r[0][1] - design_box.r[0][0] + 1);
	const int nyP = static_cast<int>(design_box.r[1][1] - design_box.r[1][0] + 1);
	const int nzP = static_cast<int>(design_box.r[2][1] - design_box.r[2][0] + 1);
	const long center_z = (design_box.r[2][0] + design_box.r[2][1]) / 2;
	const std::vector<double> design(
	    static_cast<std::size_t>(nxP) * nyP * nzP, design_value);

	const std::vector<std::string> guide_names =
	    TomlStrArray(device, "elements", "waveguides");
	std::vector<std::string> mode_regions = guide_names;
	if (auto arr = device["elements"]["background_regions"].as_array()) {
		mode_regions.clear();
		for (auto&& e : *arr) mode_regions.push_back(e.value<std::string>().value());
	}

	const Grid3<double> eps_background =
	    BuildRelativePermittivityGrid(params, device, mode_regions);
	const Grid3<double> eps_design = BuildRelativePermittivityGrid(
	    params, device, guide_names, design, nxP, nyP, nzP);

	write_plane_csv(out_dir / "vulkan_epsilon_r.csv", Nx, Ny,
	                [&](int i, int j) {
		                return eps_design(i, j, static_cast<int>(center_z));
	                });

	const std::vector<BBox> guides = [&] {
		std::vector<BBox> g;
		for (const auto& n : guide_names)
			g.push_back(BuildBoundingBox(n, device, params, false));
		return g;
	}();

	const double material_speed = 1.0 / std::sqrt(eps0 * mu0 * eps_fg);
	const double background_speed = 1.0 / std::sqrt(eps0 * mu0 * eps_bg);
	const double dt_limit =
	    courant / (std::max(material_speed, background_speed) *
	               std::sqrt(1.0 / (dx * dx) + 1.0 / (dy * dy) + 1.0 / (dz * dz)));

	const int pml_m = static_cast<int>(TomlNum(params, "pml", "m"));
	const double pml_R = TomlNum(params, "pml", "R");
	const double pml_xy = TomlNum(params, "pml", "xy");
	const double pml_z = TomlNum(params, "pml", "z");
	const double sigma_max_xy = -(pml_m + 1.0) * eps0 * background_speed *
	                            std::log(pml_R) / (2.0 * pml_xy);
	const double sigma_max_z = -(pml_m + 1.0) * eps0 * background_speed *
	                           std::log(pml_R) / (2.0 * pml_z);

	const std::vector<std::string> port_names =
	    TomlStrArray(device, "elements", "ports");
	const std::vector<std::string> source_names =
	    TomlStrArray(device, "elements", "sources");

	std::vector<MetricRow> rows;

	for (std::size_t fi = 0; fi < freqs.size(); ++fi) {
		const double f = freqs[fi];
		const double omega = 2.0 * M_PI * f;
		const double wavelength_nm = c0 / f * 1e9;

		std::map<std::string, ModeMonitor> monitors;
		for (const auto& name : port_names) {
			const BBox box = BuildModalBoundingBox(name, device, params, true);
			const PortSection section =
			    ExtractPortSection(eps_background, box, params);
			const VectorModeProfile mode = CanonicalizeVectorModePhase(
			    SolveVectorMode(section, omega, nmodes, ModeIndexOf(device, name)));
			monitors.emplace(name, MakeModeMonitor(section, {{f, mode}}, {}));
		}

		for (std::size_t ip = 0; ip < source_names.size(); ++ip) {
			const std::string& sname = source_names[ip];
			const BBox source_box =
			    BuildModalBoundingBox(sname, device, params, true);
			const PortSection source_section =
			    ExtractPortSection(eps_background, source_box, params);
			const VectorModeProfile source_mode = CanonicalizeVectorModePhase(
			    SolveVectorMode(source_section, omega, nmodes,
			                    ModeIndexOf(device, sname)));

			EquivalentModalSource src;
			src.section = source_section;
			src.freqs = {f};
			src.mode_by_freq = {{f, source_mode}};
			src.amplitudes = {cdouble(source_amplitude, 0.0)};
			src.axis = source_section.axis;
			src.fixed_index = source_section.fixed_index;
			src.range1 = source_section.range1;
			src.range2 = source_section.range2;
			src.polarity = source_section.polarity;
			src.J0 = 1.0;
			src.time_pulse = ConstantEnvelope{0.0};

			const long steps_per_cycle =
			    static_cast<long>(std::ceil(1.0 / (f * dt_limit)));
			const double dt = 1.0 / (static_cast<double>(steps_per_cycle) * f);
			const long total_steps = steps;
			const long dft_samples = dft_cycles * steps_per_cycle;

			FDTDParams fp;
			fp.whole_region_sizes = dims;
			fp.sim_region = BuildSimRegion(params);
			fp.opt_region = design_box;
			fp.opt_region_values = design;
			fp.nxP = nxP;
			fp.nyP = nyP;
			fp.nzP = nzP;
			fp.source_areas = {source_box};
			fp.waveguides_areas = guides;
			fp.sources = CreateSources({src});
			fp.monitors = monitors;
			fp.eps0 = eps0;
			fp.eps_fg = eps_fg;
			fp.eps_bg = eps_bg;
			fp.sigma_m = TomlNum(params, "material", "sigma_m");
			fp.sigma_fg = TomlNum(params, "material", "sigma_fg");
			fp.sigma_bg = TomlNum(params, "material", "sigma_bg");
			fp.mu = mu0;
			fp.pml_m = pml_m;
			fp.pml_xy = pml_xy;
			fp.pml_z = pml_z;
			fp.sigma_max_xy = sigma_max_xy;
			fp.sigma_max_z = sigma_max_z;
			fp.dx = dx;
			fp.dy = dy;
			fp.dz = dz;
			fp.dt = dt;
			fp.t = total_steps;
			fp.freqs = {f};

			auto be = MakeBackend<T>(backend);
			be->init(fp);

			bool captured = false;
			auto on_window = [&](int /*t*/, const FreqsField& ff) {
				const FreqFields& F = ff.at(f);

				std::vector<std::pair<cdouble, cdouble>> amps;
				amps.reserve(port_names.size());
				for (const auto& name : port_names)
					amps.push_back(ModalAmplitudesAtFrequency(
					    monitors.at(name), f, F));

				const cdouble incident = amps[ip].first;
				if (std::abs(incident) <=
				    std::numeric_limits<double>::epsilon())
					throw std::runtime_error(
					    "incident modal amplitude is zero");

				for (std::size_t op = 0; op < port_names.size(); ++op) {
					const cdouble S = amps[op].second / incident;
					rows.push_back(MetricRow{
					    static_cast<int>(fi) + 1, wavelength_nm, f,
					    static_cast<int>(ip) + 1, static_cast<int>(op) + 1,
					    S.real(), S.imag(), std::norm(S)});
				}

				const double scale = 2.0 / static_cast<double>(dft_samples);
				const Grid3<cdouble>& Ez = F.E[2];
				const int k = static_cast<int>(center_z);
				const std::string stem =
				    "vulkan_Ez_f" + std::to_string(fi + 1) + "_p" +
				    std::to_string(ip + 1);
				write_plane_csv(out_dir / (stem + "_real.csv"), Nx, Ny,
				                [&](int i, int j) {
					                return Ez(i, j, k).real() * scale;
				                });
				write_plane_csv(out_dir / (stem + "_imag.csv"), Nx, Ny,
				                [&](int i, int j) {
					                return Ez(i, j, k).imag() * scale;
				                });
				captured = true;
			};

			FwFDTDSimulation<T>(fp, *be, fs::path{}, std::string{}, "xy", "Ez",
			                    20, false,
			                    static_cast<int>(total_steps - dft_samples + 1),
			                    static_cast<int>(total_steps),
			                    static_cast<int>(dft_samples), on_window);

			if (!captured)
				throw std::runtime_error("DFT window produced no sample");
		}
	}

	{
		std::ofstream out(out_dir / "vulkan_metrics.csv",
		                  std::ios::binary | std::ios::trunc);
		out << "frequency_index,wavelength_nm,frequency_hz,input_port,"
		       "output_port,S_real,S_imag,T\n";
		for (const auto& r : rows)
			out << r.frequency_index << ',' << num(r.wavelength_nm) << ','
			    << num(r.frequency_hz) << ',' << r.input_port << ','
			    << r.output_port << ',' << num(r.s_real) << ',' << num(r.s_imag)
			    << ',' << num(r.transmission) << '\n';
	}

	std::cout << "END OF SIMULATION\n";
	return EXIT_SUCCESS;
}

}  // namespace ceviche_design_test

inline int RunCevicheDesignTest(const std::filesystem::path& default_dir,
                                int argc, char** argv) {
	Backend backend = Backend::taskflow;
	Precision precision = Precision::f64;
	std::filesystem::path input_dir = default_dir;
	std::filesystem::path out_base;
	bool have_out = false;

	for (int i = 1; i < argc; ++i) {
		const std::string a = argv[i];
		if (a == "--backend" && i + 1 < argc) {
			const std::string v = argv[++i];
			if (v == "sequential")
				backend = Backend::sequential;
			else if (v == "taskflow")
				backend = Backend::taskflow;
			else if (v == "vulkan")
				backend = Backend::vulkan;
			else {
				std::cerr << "unknown --backend " << v << "\n";
				return 2;
			}
		} else if (a == "--precision" && i + 1 < argc) {
			const std::string v = argv[++i];
			if (v == "f32")
				precision = Precision::f32;
			else if (v == "f64")
				precision = Precision::f64;
			else {
				std::cerr << "unknown --precision " << v << "\n";
				return 2;
			}
		} else if ((a == "--in" || a == "--dir") && i + 1 < argc) {
			input_dir = argv[++i];
		} else if (a == "--out" && i + 1 < argc) {
			out_base = argv[++i];
			have_out = true;
		} else if (!a.empty() && a[0] != '-') {
			input_dir = a;
		} else {
			std::cerr << "usage: " << (argc ? argv[0] : "design_test")
			          << " [DIR] [--in DIR] [--out DIR] "
			             "[--backend sequential|taskflow|vulkan] "
			             "[--precision f32|f64]\n";
			return 2;
		}
	}

	if (!have_out) out_base = input_dir;

	try {
		return precision == Precision::f32
		           ? ceviche_design_test::Run<float>(input_dir, out_base, backend)
		           : ceviche_design_test::Run<double>(input_dir, out_base, backend);
	} catch (const std::exception& e) {
		std::cerr << "error: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
}

}  // namespace lucuma::julia
