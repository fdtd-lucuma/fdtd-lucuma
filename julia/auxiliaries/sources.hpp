// == auxiliaries/sources.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <map>
#include <vector>

#include <Eigen/Dense>

#include "auxiliaries/modes.hpp"

namespace lucuma::julia {

// == ConstantEnvelope : envelope == 1, origin == To.
struct ConstantEnvelope {
	double To = 0.0;
	double envelope(double /*time*/) const { return 1.0; }
	double origin() const { return To; }
};

// == EquivalentModalSource (broadband).
struct EquivalentModalSource {
	PortSection section;
	std::vector<double> freqs;                    // Hz, DFT-bin aligned
	std::map<double, VectorModeProfile> mode_by_freq;
	std::vector<cdouble> amplitudes;              // spectral weights, max |.| == 1
	Axis axis = Axis::X;
	long fixed_index = 0;
	IdxRange range1, range2;
	int polarity = 1;
	double J0 = 1.0;
	ConstantEnvelope time_pulse{};
};

// == DFTBinFrequencies : bins of the length-`sample_count` DFT window that lie
// inside [fmin, fmax].
std::vector<double> DFTBinFrequencies(double fmin, double fmax, double dt,
                                      long sample_count);

// == BuildBroadbandModalSource.
EquivalentModalSource BuildBroadbandModalSource(const TrackedModeFamily& family,
                                                double dt, long sample_count,
                                                double center_frequency,
                                                double spectral_width,
                                                double time_origin,
                                                double J0 = 1.0);

// == MagneticSheetIndex.
inline long MagneticSheetIndex(const PortSection& s) {
	return s.fixed_index - (s.polarity > 0 ? 1 : 0);
}

// == CrossWithPortNormal : (polarity * e_axis) x F  ->  (Gx, Gy, Gz).
std::array<Eigen::MatrixXcd, 3> CrossWithPortNormal(Axis axis, int polarity,
                                                    const Eigen::MatrixXcd& Fx,
                                                    const Eigen::MatrixXcd& Fy,
                                                    const Eigen::MatrixXcd& Fz);

// == EquivalentSurfaceCurrents : (J, M) sheets for one mode on its port.
struct SurfaceCurrents {
	std::array<Eigen::MatrixXcd, 3> J;  // n x H
	std::array<Eigen::MatrixXcd, 3> M;  // -n x E
};
SurfaceCurrents EquivalentSurfaceCurrents(const VectorModeProfile& mode,
                                          const PortSection& section);

}  // namespace lucuma::julia
