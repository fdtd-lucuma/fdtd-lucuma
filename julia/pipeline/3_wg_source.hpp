// == pipeline/3_wg_source.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "auxiliaries/funcs.hpp"
#include "auxiliaries/modes.hpp"
#include "auxiliaries/sources.hpp"

namespace lucuma::vulkan {

BBox BuildModalBoundingBox(const std::string& name, const Params& device,
                           const Params& params, bool with_direction = true);

struct BroadbandBand {
	double fmin, fmax, center, spectral_width;
};
// == BroadbandModalBand : the frequency band a broadband modal source spans,
// derived from the optimisation frequencies + simulation duration.
BroadbandBand BroadbandModalBand(const std::vector<double>& freqs,
                                 double duration, double alpha,
                                 double truncation = 1e-8);

// Loads P (sized from device["P"]["dim"]) and rasterises win/wout + the design
// region into a relative-permittivity grid.
Grid3<double> BuildPermGrid(const std::filesystem::path& params_path,
                            const std::filesystem::path& device_path,
                            const std::filesystem::path& region_path);

// == DefineModalSources (solver = :vector, broadband = true). One broadband
// equivalent-current source per name, in name order.
std::vector<EquivalentModalSource> DefineModalSources(
    const Params& params, const Params& device,
    const std::vector<std::string>& source_names, const Grid3<double>& eps_grid,
    long transverse_pad = 0, double J0 = 1.0, int nmodes = 4,
    int mode_index = 1, double alpha = 0.1, int modal_sample_count = 9,
    double spectral_truncation = 1e-8);

// == DefineModeMonitor (solver = :vector). Modes solved directly at each
// optimisation frequency; `targets` read from device[name] if present.
ModeMonitor DefineModeMonitor(const Params& params, const Params& device,
                              const std::string& monitor_name,
                              const Grid3<double>& eps_grid,
                              long transverse_pad = 0, int nmodes = 4,
                              int mode_index = 1);

}  // namespace lucuma::vulkan
