// == pipeline/3_wg_source.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pipeline/3_wg_source.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace lucuma::julia {

BroadbandBand BroadbandModalBand(const std::vector<double>& freqs,
                                 double duration, double alpha,
                                 double truncation) {
	const double fmn = *std::min_element(freqs.begin(), freqs.end());
	const double fmx = *std::max_element(freqs.begin(), freqs.end());
	const double center = 0.5 * (fmn + fmx);
	double edge = 0.0;
	for (double f : freqs) edge = std::max(edge, std::abs(f - center));

	const double minimum_width = 1.6 / duration;
	double spectral_width = edge > 0.0
	                            ? edge / std::sqrt(2.0 * std::log(1.0 / alpha))
	                            : minimum_width;
	spectral_width = std::max(spectral_width, minimum_width);

	const double radius = spectral_width * std::sqrt(2.0 * std::log(1.0 / truncation));
	return {std::max(std::numeric_limits<double>::epsilon(), center - radius),
	        center + radius, center, spectral_width};
}

BBox BuildModalBoundingBox(const std::string& name, const Params& device,
                           const Params& params, bool with_direction) {
	static constexpr std::array<std::pair<const char*, const char*>, 3> DIMS{
	    {{"x", "dx"}, {"y", "dy"}, {"z", "dz"}}};

	BBox b;
	b.name = name;
	for (int d = 0; d < 3; ++d) {
		const auto& [coord, step] = DIMS[d];
		auto arr = device[name][coord].as_array();
		const auto rng = ValuesToIndx((*arr)[0].value<double>().value(),
		                              (*arr)[1].value<double>().value(),
		                              TomlNum(params, "domain", step));
		b.r[d] = {rng[0], rng[1]};
	}
	if (with_direction)
		b.direction = TomlStr(device, name, "direction");
	return b;
}

Grid3<double> BuildPermGrid(const std::filesystem::path& params_path,
                            const std::filesystem::path& device_path,
                            const std::filesystem::path& region_path) {
	const Params params = LoadParams(params_path);
	const Params device = LoadParams(device_path);

	auto dim = device["P"]["dim"].as_array();
	const int nxP = static_cast<int>((*dim)[0].value<int64_t>().value());
	const int nyP = static_cast<int>((*dim)[1].value<int64_t>().value());
	const int nzP = static_cast<int>((*dim)[2].value<int64_t>().value());

	const std::vector<double> P = LoadP(nxP, nyP, nzP, region_path);

	return BuildRelativePermittivityGrid(params, device, {"win", "wout"}, P, nxP,
	                                     nyP, nzP);
}

std::vector<EquivalentModalSource> DefineModalSources(
    const Params& params, const Params& device,
    const std::vector<std::string>& source_names, const Grid3<double>& eps_grid,
    long transverse_pad, double J0, int nmodes, int mode_index, double alpha,
    int modal_sample_count, double spectral_truncation) {

	const std::vector<double> freqs_hz =
	    TomlF64Array(params, "simulation", "freqs");
	const long t = TomlInt(params, "simulation", "t");
	const double dt = TomlNum(params, "simulation", "dt");
	const double duration = static_cast<double>(t) * dt;

	std::vector<EquivalentModalSource> out;
	out.reserve(source_names.size());

	for (const auto& name : source_names) {
		const BBox bbox = BuildModalBoundingBox(name, device, params, true);
		const PortSection section =
		    ExtractPortSection(eps_grid, bbox, params, transverse_pad);

		const BroadbandBand band =
		    BroadbandModalBand(freqs_hz, duration, alpha, spectral_truncation);
		const std::vector<double> sample_frequencies = ChebyshevFrequencySamples(
		    band.fmin, band.fmax, modal_sample_count);
		const TrackedModeFamily family = BuildTrackedModeFamily(
		    section, sample_frequencies, nmodes, mode_index);

		out.push_back(BuildBroadbandModalSource(
		    family, dt, t, band.center, band.spectral_width, 0.5 * duration,
		    J0));
	}
	return out;
}

ModeMonitor DefineModeMonitor(const Params& params, const Params& device,
                              const std::string& monitor_name,
                              const Grid3<double>& eps_grid, long transverse_pad,
                              int nmodes, int mode_index) {
	const BBox bbox = BuildModalBoundingBox(monitor_name, device, params, true);
	const PortSection section =
	    ExtractPortSection(eps_grid, bbox, params, transverse_pad);

	std::vector<double> targets;
	if (auto arr = device[monitor_name]["targets"].as_array())
		for (auto&& e : *arr) targets.push_back(e.value<double>().value());

	std::map<double, VectorModeProfile> modes_by_freq;
	for (double f : TomlF64Array(params, "simulation", "freqs"))
		modes_by_freq.emplace(
		    f, SolveVectorMode(section, 2.0 * M_PI * f, nmodes, mode_index));

	return MakeModeMonitor(section, std::move(modes_by_freq), std::move(targets));
}

}  // namespace lucuma::julia
