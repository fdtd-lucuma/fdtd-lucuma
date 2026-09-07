// == pipeline/0_loads.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace lucuma::vulkan {

// LoadInputs(base_path, design): create input/<design>/ and copy the base
// TOMLs into it if they are not there yet.
void LoadInputs(const std::filesystem::path& base_path, const std::string& design);

struct Stages {
	std::string stage;  // "cont" | "disc" | "fab" | "completed"
	long beta = 0;
	long it = 1;
	long step = 20;
	long cont = 0, disc = 0, fab = 0;                 // iteration counts per stage
	std::map<std::string, std::array<long, 2>> beta_range;
	double bin_threshold = 0.5;
};
Stages StagesParams(const std::filesystem::path& params_path);

struct Elements {
	std::vector<std::string> waveguides_in;
	std::vector<std::string> waveguides_out;
	std::vector<std::string> monitors_in;
	std::vector<std::string> monitors_out;
	std::vector<std::string> sources;
};
Elements ElementsParams(const std::filesystem::path& device_path);

struct Filters {
	double radius = 0.0;
	std::array<long, 3> neighbours{0, 0, 0};
	double eta_nom = 0.5, eta_ero = 0.55, eta_dil = 0.45;
};
// Mirrors FiltersParams: when stage == "cont" the cell radius is recomputed
// from radius/dx,dy,dz and written back into params.toml via UpdateNeighbours.
Filters FiltersParams(const std::filesystem::path& params_path,
                      const std::string& stage);

struct Optimizer {
	std::string algorithm;                   // lowercased params["optimizer"]["algorithm"]
	std::map<std::string, double> params;    // the optimizer.toml[algorithm] table
};
Optimizer OptimizerParams(const std::filesystem::path& params_path,
                          const std::filesystem::path& optimizer_path);

struct Debug {
	std::string plane = "xy";
	std::string field = "Ey";
	long step = 20;
	std::string result_plane = "xy";
};
Debug DebugParams(const std::filesystem::path& debug_path);

}  // namespace lucuma::vulkan
