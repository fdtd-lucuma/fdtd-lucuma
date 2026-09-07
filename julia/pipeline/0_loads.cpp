// == pipeline/0_loads.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pipeline/0_loads.hpp"

#include <cctype>
#include <cmath>

#include "auxiliaries/funcs.hpp"

namespace fs = std::filesystem;

namespace lucuma::vulkan {

void LoadInputs(const fs::path& base_path, const std::string& design) {
	const fs::path design_path = base_path / design;
	fs::create_directories(design_path);

	for (const char* file :
	     {"params.toml", "device.toml", "optimizer.toml", "debug.toml"}) {
		const fs::path dst = design_path / file;
		if (!fs::exists(dst)) fs::copy_file(base_path / file, dst);
	}
}

Stages StagesParams(const fs::path& params_path) {
	const Params p = LoadParams(params_path);
	Stages s;
	s.stage = TomlStr(p, "simulation", "stage");
	s.beta = TomlInt(p, "simulation", "beta");
	s.it = TomlInt(p, "simulation", "it");
	s.step = TomlInt(p, "simulation", "step");
	s.cont = TomlInt(p, "simulation", "cont");
	s.disc = TomlInt(p, "simulation", "disc");
	s.fab = TomlInt(p, "simulation", "fab");
	s.bin_threshold = TomlNum(p, "simulation", "bin_threshold");
	if (auto br = p["simulation"]["beta_range"].as_table())
		for (auto&& [k, v] : *br)
			if (auto a = v.as_array(); a && a->size() == 2)
				s.beta_range[std::string(k.str())] = {
				    static_cast<long>((*a)[0].value<int64_t>().value_or(0)),
				    static_cast<long>((*a)[1].value<int64_t>().value_or(0))};
	return s;
}

Elements ElementsParams(const fs::path& device_path) {
	const Params d = LoadParams(device_path);
	Elements e;
	e.waveguides_in = TomlStrArray(d, "elements", "waveguides_in");
	e.waveguides_out = TomlStrArray(d, "elements", "waveguides_out");
	e.monitors_in = TomlStrArray(d, "elements", "monitors_in");
	e.monitors_out = TomlStrArray(d, "elements", "monitors_out");
	e.sources = TomlStrArray(d, "elements", "sources");
	return e;
}

Filters FiltersParams(const fs::path& params_path, const std::string& stage) {
	const Params p = LoadParams(params_path);
	Filters f;
	f.radius = TomlF64(p, "filters", "radius");
	f.eta_nom = p["filters"]["eta"]["nom"].value<double>().value_or(0.5);
	f.eta_ero = p["filters"]["eta"]["ero"].value<double>().value_or(0.55);
	f.eta_dil = p["filters"]["eta"]["dil"].value<double>().value_or(0.45);

	if (stage != "cont") {
		auto nb = TomlIntArray(p, "filters", "neighbours");
		f.neighbours = {nb.at(0), nb.at(1), nb.at(2)};
	} else {
		const double dx = TomlF64(p, "domain", "dx");
		const double dy = TomlF64(p, "domain", "dy");
		const double dz = TomlF64(p, "domain", "dz");
		f.neighbours = {static_cast<long>(std::ceil(f.radius / dx)),
		                static_cast<long>(std::ceil(f.radius / dy)),
		                static_cast<long>(std::ceil(f.radius / dz))};
		UpdateNeighbours(params_path,
		                 {static_cast<int>(f.neighbours[0]),
		                  static_cast<int>(f.neighbours[1]),
		                  static_cast<int>(f.neighbours[2])});
	}
	return f;
}

Optimizer OptimizerParams(const fs::path& params_path,
                          const fs::path& optimizer_path) {
	const Params p = LoadParams(params_path);
	Optimizer o;
	o.algorithm = TomlStr(p, "optimizer", "algorithm");
	for (auto& c : o.algorithm) c = static_cast<char>(std::tolower(c));

	const Params opt = LoadParams(optimizer_path);
	if (auto tbl = opt[o.algorithm].as_table()) {
		for (auto&& [k, v] : *tbl) {
			if (auto d = v.value<double>())
				o.params[std::string(k.str())] = *d;
			else if (auto i = v.value<int64_t>())
				o.params[std::string(k.str())] = static_cast<double>(*i);
		}
	}
	return o;
}

Debug DebugParams(const fs::path& debug_path) {
	const Params d = LoadParams(debug_path);
	Debug dbg;
	dbg.plane = TomlStr(d, "fdtd", "plane");
	dbg.field = TomlStr(d, "fdtd", "field");
	dbg.step = TomlInt(d, "fdtd", "step");
	dbg.result_plane = d["result"]["plane"].value<std::string>().value_or("xy");
	return dbg;
}

}  // namespace lucuma::vulkan
