// == pipeline/2_device.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pipeline/2_device.hpp"

#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include "auxiliaries/funcs.hpp"

namespace lucuma::julia {

std::array<int, 3> DefineDevice(const std::filesystem::path& params_path,
                                const std::filesystem::path& device_path,
                                const std::filesystem::path& output_path,
                                bool p_ones) {
	const Params params = LoadParams(params_path);
	const Params device = LoadParams(device_path);

	const auto Px = TomlPair(device, "P", "x");
	const auto Py = TomlPair(device, "P", "y");
	const auto Pz = TomlPair(device, "P", "z");

	const double dx = TomlNum(params, "domain", "dx");
	const double dy = TomlNum(params, "domain", "dy");
	const double dz = TomlNum(params, "domain", "dz");

	const int nx = static_cast<int>(julia_round((Px[1] - Px[0]) / dx));
	const int ny = static_cast<int>(julia_round((Py[1] - Py[0]) / dy));
	const int nz = static_cast<int>(julia_round((Pz[1] - Pz[0]) / dz));

	InitP(nx, ny, nz, output_path, p_ones);

	// Append `dim = [...]` to device.toml, dropping any prior dim line.
	const std::regex dim_line(R"(^\s*dim\s*=)");
	std::vector<std::string> lines;
	{
		std::ifstream in(device_path);
		std::string line;
		while (std::getline(in, line)) {
			if (!line.empty() && line.back() == '\r') line.pop_back();
			if (!std::regex_search(line, dim_line)) lines.push_back(std::move(line));
		}
	}
	std::ofstream out(device_path, std::ios::binary | std::ios::trunc);
	for (const auto& l : lines) out << l << '\n';
	out << "dim = [" << nx << ", " << ny << ", " << nz << "]\n";

	return {nx, ny, nz};
}

}  // namespace lucuma::julia
