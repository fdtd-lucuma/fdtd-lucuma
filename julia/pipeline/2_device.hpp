// == pipeline/2_device.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <filesystem>

namespace lucuma::vulkan {

// DefineDevice: size the optimisation region P from device["P"] extents and
// the domain steps, write output/<layout>/P.csv (ones when p_ones), and append
// `dim = [nx, ny, nz]` to device.toml (prior dim lines dropped first).
// Returns the P dimensions.
std::array<int, 3> DefineDevice(const std::filesystem::path& params_path,
                                const std::filesystem::path& device_path,
                                const std::filesystem::path& output_path,
                                bool p_ones = false);

}  // namespace lucuma::vulkan
