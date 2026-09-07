// == pipeline/1_params.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>

namespace lucuma::julia {

// DefineParams: derive cfg, cbg, dt (Courant), sigma_max_xy, sigma_max_z from
// the material / domain / pml tables and write the five values back into
// params.toml (existing lines for those keys are dropped first).
void DefineParams(const std::filesystem::path& params_path);

}  // namespace lucuma::julia
