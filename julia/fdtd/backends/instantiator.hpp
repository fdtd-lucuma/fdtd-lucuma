// No direct Julia counterpart — mirrors fdtd-lucuma/src/services/backends/instantiator.cppm.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "fdtd/backends/i_backend.hpp"
#include "pipeline/0_loads.hpp"      // Debug
#include "pipeline/7_forw_fdtd.hpp"
#include "utils/precision.hpp"

namespace lucuma::julia {

enum class Backend { sequential, taskflow, vulkan };

// Construct a backend for scalar type T (float | double). Throws for vulkan + f64.
template <class T>
std::unique_ptr<IFdtdBackend<T>> MakeBackend(Backend b);

// Paths / snapshot knobs for the calibration + forward runs.
struct ForwardRunOptions {
	std::filesystem::path vis_path;              // "$vis_dir/fw"
	std::filesystem::path norm_path;             // "$input_dir/norm.toml"
	std::string proj = "1_1_1";                  // RunForwardFDTD proj arg
	std::string calibration_tag = "calibration";
	std::string forward_tag = "1_1_1";           // RunForwardFDTD additional arg
	std::string snapshot_plane = "xy";
	std::string snapshot_field = "Ey";
	int step = 20;
	bool save_result = false;
};

// Populate ForwardRunOptions from the run root / layout + debug.toml settings.
ForwardRunOptions MakeForwardRunOptions(const std::filesystem::path& root,
                                        const std::string& layout,
                                        const Debug& debug);

// Build the (backend, precision) backend, init, calibrate incident power, run forward.
ForwardResult RunForwardOnBackend(Backend backend, Precision precision,
                                  FDTDParams& params,
                                  const std::vector<std::string>& monitors_in,
                                  const std::vector<std::string>& monitors_out,
                                  const ForwardRunOptions& options = {});

}  // namespace lucuma::julia
