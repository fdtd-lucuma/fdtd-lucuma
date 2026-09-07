// == pipeline/4_fdtd_setup.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "auxiliaries/funcs.hpp"
#include "auxiliaries/modes.hpp"
#include "auxiliaries/sources.hpp"
#include "fdtd/backends/i_backend.hpp"
#include "fdtd/fw_sim.hpp"
#include "fdtd/params.hpp"
#include "fdtd/utils.hpp"

namespace lucuma::julia {

// [Nx, Ny, Nz] = round(L / d)
std::array<int, 3> WholeRegionSizes(const Params& params);

// PML-inset simulation region [[x_lo,x_hi],[y_lo,y_hi],[z_lo,z_hi]] (1-based).
SimRegion BuildSimRegion(const Params& params);

// device box -> BBox with x/y/z index ranges (and the direction string when
// with_direction).
BBox BuildBoundingBox(const std::string& name, const Params& device,
                      const Params& params, bool with_direction = true);

// Assemble FDTDParams from the input files + element names + the broadband
// modal sources, and fill the update coefficients. Monitor mode data arrives
// in a later phase; here only monitor bounding boxes are recorded.
FDTDParams FDTDSetUp(const std::filesystem::path& params_path,
                     const std::filesystem::path& device_path,
                     const std::filesystem::path& region_path,
                     const std::vector<EquivalentModalSource>& modal_sources,
                     const std::vector<std::string>& source_area_names,
                     const std::vector<std::string>& waveguide_names,
                     std::map<std::string, ModeMonitor> monitors,
                     const std::vector<std::string>& monitor_names);

inline constexpr int SOURCE_NORMALIZATION_SCHEMA_VERSION = 2;

// FNV-1a hex digest of the calibration-relevant FDTDParams + monitor data.
std::string SourceNormalizationSignature(
    const FDTDParams& params, const std::vector<std::string>& monitor_names);

bool SourceNormalizationCacheIsCurrent(const std::filesystem::path& norm_path,
                                       const std::string& expected_signature);

SourceNormalization LoadSourceNormalization(
    const std::filesystem::path& norm_path);

void SaveSourceNormalization(const SourceNormalization& sn,
                             const std::filesystem::path& norm_path,
                             const std::string& signature);

// == CalibrateIncidentPower : reuse norm.toml on a signature match, else recompute + cache.
template <class T>
SourceNormalization CalibrateIncidentPower(
    const FDTDParams& params, IFdtdBackend<T>& backend,
    const std::vector<std::string>& monitors_in,
    const std::filesystem::path& vis_path = {},
    const std::filesystem::path& norm_path = {},
    const std::string& additional = {}, const std::string& snapshot_plane = "xy",
    const std::string& snapshot_field = "Ey") {
	const std::string signature =
	    SourceNormalizationSignature(params, monitors_in);

	if (!norm_path.empty() &&
	    SourceNormalizationCacheIsCurrent(norm_path, signature))
		return LoadSourceNormalization(norm_path);

	const FreqsField calibration_fields = FwFDTDSimulation<T>(
	    params, backend, vis_path, additional, snapshot_plane, snapshot_field);
	SourceNormalization sn = ComputeSourceNormalization(
	    params.freqs, params.monitors, calibration_fields, monitors_in);

	if (!norm_path.empty()) SaveSourceNormalization(sn, norm_path, signature);
	return sn;
}

}  // namespace lucuma::julia
