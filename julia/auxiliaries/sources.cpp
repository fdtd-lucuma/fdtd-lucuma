// == auxiliaries/sources.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auxiliaries/sources.hpp"

#include <algorithm>
#include <cmath>

namespace lucuma::vulkan {

std::vector<double> DFTBinFrequencies(double fmin, double fmax, double dt,
                                      long sample_count) {
	const double spacing = 1.0 / (static_cast<double>(sample_count) * dt);
	const long first_bin = static_cast<long>(std::ceil(fmin / spacing));
	const long last_bin = static_cast<long>(std::floor(fmax / spacing));

	std::vector<double> out;
	for (long b = first_bin; b <= last_bin; ++b) out.push_back(spacing * b);
	return out;
}

EquivalentModalSource BuildBroadbandModalSource(const TrackedModeFamily& family,
                                                double dt, long sample_count,
                                                double center_frequency,
                                                double spectral_width,
                                                double time_origin, double J0) {
	const double fmin = family.frequencies.front();
	const double fmax = family.frequencies.back();

	const std::vector<double> frequencies =
	    DFTBinFrequencies(fmin, fmax, dt, sample_count);

	EquivalentModalSource src;
	src.section = family.section;
	src.freqs = frequencies;
	for (double f : frequencies)
		src.mode_by_freq.emplace(f, InterpolateTrackedMode(family, f));

	src.amplitudes.resize(frequencies.size());
	double max_amp = 0.0;
	for (std::size_t i = 0; i < frequencies.size(); ++i) {
		const double x = (frequencies[i] - center_frequency) / spectral_width;
		src.amplitudes[i] = cdouble(std::exp(-0.5 * x * x), 0.0);
		max_amp = std::max(max_amp, std::abs(src.amplitudes[i]));
	}
	for (auto& a : src.amplitudes) a /= max_amp;

	src.axis = family.section.axis;
	src.fixed_index = family.section.fixed_index;
	src.range1 = family.section.range1;
	src.range2 = family.section.range2;
	src.polarity = family.section.polarity;
	src.J0 = J0;
	src.time_pulse = ConstantEnvelope{time_origin};
	return src;
}

std::array<Eigen::MatrixXcd, 3> CrossWithPortNormal(Axis axis, int polarity,
                                                    const Eigen::MatrixXcd& Fx,
                                                    const Eigen::MatrixXcd& Fy,
                                                    const Eigen::MatrixXcd& Fz) {
	const double p = static_cast<double>(polarity);
	const Eigen::MatrixXcd zero = Eigen::MatrixXcd::Zero(Fx.rows(), Fx.cols());
	switch (axis) {
		case Axis::X: return {zero, -p * Fz, p * Fy};
		case Axis::Y: return {p * Fz, zero, -p * Fx};
		default:      return {-p * Fy, p * Fx, zero};  // Axis::Z
	}
}

SurfaceCurrents EquivalentSurfaceCurrents(const VectorModeProfile& mode,
                                          const PortSection& section) {
	// Positive propagation: H is taken on the opposite Yee sheet.
	const cdouble h_to_sheet =
	    section.polarity > 0
	        ? std::exp(cdouble(0.0, 1.0) * mode.beta * section.delta_normal)
	        : cdouble(1.0, 0.0);

	const Eigen::MatrixXcd Hx = mode.Hx * h_to_sheet;
	const Eigen::MatrixXcd Hy = mode.Hy * h_to_sheet;
	const Eigen::MatrixXcd Hz = mode.Hz * h_to_sheet;

	SurfaceCurrents sc;
	sc.J = CrossWithPortNormal(section.axis, section.polarity, Hx, Hy, Hz);
	const auto nxE = CrossWithPortNormal(section.axis, section.polarity, mode.Ex,
	                                     mode.Ey, mode.Ez);
	sc.M = {-nxE[0], -nxE[1], -nxE[2]};
	return sc;
}

}  // namespace lucuma::vulkan
