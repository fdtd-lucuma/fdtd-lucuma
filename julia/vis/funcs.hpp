// == vis/funcs.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <string>

#include "auxiliaries/array.hpp"

namespace lucuma::julia {

// writedlm(path, matrix, ',') : one row per line, comma separated, row-major.
void WriteMatrixCSV(const std::filesystem::path& path, const Mat2<double>& m);

// == FDTDSnapshot : mid-plane slice -> "<field>_<plane>_<additional>_<t:06d>.csv".
void FDTDSnapshot(const Grid3<double>& field, const std::string& field_name,
                  int t, const std::string& additional,
                  const std::filesystem::path& vis_path,
                  const std::string& plane = "xy");

}  // namespace lucuma::julia
