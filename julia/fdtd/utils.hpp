// == fdtd/utils.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <complex>
#include <vector>

#include "auxiliaries/array.hpp"
#include "auxiliaries/funcs.hpp"
#include "auxiliaries/sources.hpp"

namespace lucuma::julia {

// sim_region is [[x_lo,x_hi],[y_lo,y_hi],[z_lo,z_hi]] (1-based inclusive).
using SimRegion = std::array<std::array<long, 2>, 3>;

struct Bounds {
	long x_min, x_max, y_min, y_max, z_min, z_max;
};

inline Bounds BoundsRegion(const BBox& b) {
	return {b.r[0][0], b.r[0][1], b.r[1][0], b.r[1][1], b.r[2][0], b.r[2][1]};
}

inline Bounds BoundsSimRegion(const SimRegion& s) {
	return {s[0][0], s[0][1], s[1][0], s[1][1], s[2][0], s[2][1]};
}

// ---------------------------------------------------------------------------
// coefficient fill  (== fdtd/utils.jl CoeffsParams / GetEps / GetSigma / ...)
// ---------------------------------------------------------------------------

struct CoeffsParams {
	double dt;
	double eps_fg;    // material eps_fg * eps0  (absolute)
	double eps_bg;    // material eps_bg * eps0  (absolute)
	double sigma_fg;
	double sigma_bg;
	int m;
	double sigma_max_xy;
	double sigma_max_z;
	Bounds sim_region_bounds;
	Bounds opt_region_bounds;
	std::vector<Bounds> waveguide_bounds;
	// design density, flattened column-major, sized by the opt-region bbox.
	const std::vector<double>* opt_region_values = nullptr;
	int nxP = 0, nyP = 0, nzP = 0;
};

// eps / sigma / pml / time here mirror the small Dicts assembled in FDTDSetUp.
CoeffsParams MakeCoeffsParams(double dt, double eps0, double eps_fg,
                              double eps_bg, double sigma_fg, double sigma_bg,
                              int m, double sigma_max_xy, double sigma_max_z,
                              const SimRegion& sim_region,
                              const BBox& opt_region,
                              const std::vector<BBox>& waveguide_areas,
                              const std::vector<double>& opt_region_values,
                              int nxP, int nyP, int nzP);

double GetSigmaPML(long i, long j, long k, const Bounds& sim, int m,
                   double sigma_max_xy, double sigma_max_z);

double GetEps(long i, long j, long k, const std::vector<Bounds>& wg,
              const Bounds& opt, const std::vector<double>& P, int nxP, int nyP,
              double eps_fg, double eps_bg);

double GetSigma(long i, long j, long k, const std::vector<Bounds>& wg,
                const Bounds& opt, const std::vector<double>& P, int nxP,
                int nyP, double sigma_fg, double sigma_bg);

// ---------------------------------------------------------------------------
// FDTD source (== fdtd/utils.jl Source): equivalent J/M surface-current sheets
// stacked over frequency.
// ---------------------------------------------------------------------------

struct Source {
	// each vector is size1*size2*nfreqs, column-major:
	//   idx = (a-1) + size1*((b-1) + size2*n)   a in 1..size1, b in 1..size2
	std::vector<std::complex<double>> Jx, Jy, Jz, Mx, My, Mz;
	std::vector<double> omegas;  // 2*pi * freqs, length nfreqs
	int nfreqs = 0;
	Axis axis = Axis::X;
	long electric_fixed_index = 0;
	long magnetic_fixed_index = 0;
	long r1_lo = 1, r2_lo = 1;
	int size1 = 0, size2 = 0;
	double inv_delta_normal = 0.0;
	double J0 = 1.0;
	double time_origin = 0.0;  // ConstantEnvelope.To ; envelope == 1
};

// == CreateSources : one Source per broadband modal source (source-name order).
std::vector<Source> CreateSources(
    const std::vector<EquivalentModalSource>& sources);

}  // namespace lucuma::julia
