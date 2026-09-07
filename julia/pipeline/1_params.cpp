// == pipeline/1_params.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pipeline/1_params.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include "auxiliaries/funcs.hpp"

namespace lucuma::julia {

void DefineParams(const std::filesystem::path& params_path) {
	const Params p = LoadParams(params_path);

	const double eps0 = TomlNum(p, "material", "eps0");
	const double eps_fg = TomlNum(p, "material", "eps_fg");
	const double eps_bg = TomlNum(p, "material", "eps_bg");
	const double mu0 = TomlNum(p, "material", "mu0");

	const double dx = TomlNum(p, "domain", "dx");
	const double dy = TomlNum(p, "domain", "dy");
	const double dz = TomlNum(p, "domain", "dz");

	const double m = TomlNum(p, "pml", "m");
	const double R = TomlNum(p, "pml", "R");
	const double pml_xy = TomlNum(p, "pml", "xy");
	const double pml_z = TomlNum(p, "pml", "z");

	const double cfg = 1.0 / std::sqrt(eps_fg * eps0 * mu0);
	const double cbg = 1.0 / std::sqrt(eps_bg * eps0 * mu0);
	const double c = std::max(cfg, cbg);

	const double dt =
	    1.0 / (c * std::sqrt((1.0 / dx) * (1.0 / dx) + (1.0 / dy) * (1.0 / dy) +
	                         (1.0 / dz) * (1.0 / dz)));

	const double sigma_max_xy =
	    -(m + 1.0) * eps0 * cbg * std::log(R) / (2.0 * pml_xy);
	const double sigma_max_z =
	    -(m + 1.0) * eps0 * cbg * std::log(R) / (2.0 * pml_z);

	// Rewrite params.toml: drop any prior lines for these keys, then append.
	const std::regex precalc(
	    R"(^\s*(cfg|cbg|dt|sigma_max_xy|sigma_max_z)\s*=)");

	std::vector<std::string> lines;
	{
		std::ifstream in(params_path);
		std::string line;
		while (std::getline(in, line)) {
			if (!line.empty() && line.back() == '\r') line.pop_back();
			if (!std::regex_search(line, precalc)) lines.push_back(std::move(line));
		}
	}

	std::ofstream out(params_path, std::ios::binary | std::ios::trunc);
	for (const auto& l : lines) out << l << '\n';
	out << "cfg          = " << julia_repr(cfg)
	    << "  # speed of light in foreground: [m/s]\n";
	out << "cbg          = " << julia_repr(cbg)
	    << "  # speed of light in background: [m/s]\n";
	out << "dt           = " << julia_repr(dt) << "  # time step: [s]\n";
	out << "sigma_max_xy = " << julia_repr(sigma_max_xy)
	    << "  # max PML conductivity in x,y: [S/m]\n";
	out << "sigma_max_z  = " << julia_repr(sigma_max_z)
	    << "  # max PML conductivity in z: [S/m]\n";
}

}  // namespace lucuma::julia
