// == fdtd/params.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <array>
#include <map>
#include <string>
#include <vector>

#include "auxiliaries/array.hpp"
#include "auxiliaries/funcs.hpp"
#include "auxiliaries/modes.hpp"
#include "fdtd/utils.hpp"

namespace lucuma::julia {

// == ChhChe : coefficient math in double, stored as T (== fdtd/params.jl ChhChe).
template <class T>
void ChhChe(Grid3<T>& Chh, Grid3<T>& Che, long i_lo, long i_hi, long j_lo,
            long j_hi, long k_lo, long k_hi, const Bounds& sim, int m,
            double sigma_max_xy, double sigma_max_z, double mu, double dt,
            double eps_bg) {
	for (long k = k_lo; k <= k_hi; ++k)
		for (long j = j_lo; j <= j_hi; ++j)
			for (long i = i_lo; i <= i_hi; ++i) {
				const double sm =
				    GetSigmaPML(i, j, k, sim, m, sigma_max_xy, sigma_max_z) * mu /
				    eps_bg;
				const double denom_h = 1.0 + sm * dt / (2.0 * mu);
				const int I = int(i), J = int(j), K = int(k);
				Chh(I, J, K) =
				    static_cast<T>((1.0 - sm * dt / (2.0 * mu)) / denom_h);
				Che(I, J, K) = static_cast<T>((dt / mu) / denom_h);
			}
}

// == CeeCeh (== fdtd/params.jl CeeCeh).
template <class T>
void CeeCeh(Grid3<T>& Cee, Grid3<T>& Ceh, long i_lo, long i_hi, long j_lo,
            long j_hi, long k_lo, long k_hi, const std::vector<Bounds>& wg,
            const Bounds& opt, const std::vector<double>& P, int nxP, int nyP,
            double eps_fg, double eps_bg, double sigma_fg, double sigma_bg,
            const Bounds& sim, int m, double sigma_max_xy, double sigma_max_z,
            double dt) {
	for (long k = k_lo; k <= k_hi; ++k)
		for (long j = j_lo; j <= j_hi; ++j)
			for (long i = i_lo; i <= i_hi; ++i) {
				const double eps_val =
				    GetEps(i, j, k, wg, opt, P, nxP, nyP, eps_fg, eps_bg);
				const double sigma_val =
				    GetSigma(i, j, k, wg, opt, P, nxP, nyP, sigma_fg, sigma_bg) +
				    GetSigmaPML(i, j, k, sim, m, sigma_max_xy, sigma_max_z);
				const double denom_e = 1.0 + sigma_val * dt / (2.0 * eps_val);
				const int I = int(i), J = int(j), K = int(k);
				Cee(I, J, K) = static_cast<T>(
				    (1.0 - sigma_val * dt / (2.0 * eps_val)) / denom_e);
				Ceh(I, J, K) = static_cast<T>((dt / eps_val) / denom_e);
			}
}

struct FDTDParams {
	std::array<int, 3> whole_region_sizes{0, 0, 0};  // Nx, Ny, Nz
	SimRegion sim_region{};
	BBox opt_region{};                     // "P" box, no direction
	std::vector<double> opt_region_values;  // flat column-major, nxP*nyP*nzP
	int nxP = 0, nyP = 0, nzP = 0;
	std::vector<BBox> source_areas;
	std::vector<BBox> waveguides_areas;
	std::vector<BBox> monitors_areas;

	std::vector<Source> sources;                     // == FDTDParams.sources
	std::map<std::string, ModeMonitor> monitors;     // == FDTDParams.monitors

	// constants (material relative values + eps0; mu is mu0)
	double eps0 = 0, eps_fg = 0, eps_bg = 0;
	double sigma_m = 0, sigma_fg = 0, sigma_bg = 0;
	double mu = 0;
	int pml_m = 0;
	double pml_xy = 0, pml_z = 0, sigma_max_xy = 0, sigma_max_z = 0;
	double dx = 0, dy = 0, dz = 0;
	double dt = 0;
	long t = 0;
	std::vector<double> freqs;
};

// (Re)allocate + fill coefficient grids over the whole grid (== the FDTDParams
// constructor body: ChhChe over 1..N-1, CeeCeh over 2..N).
template <class T>
void FillCoeffsWholeGrid(Grid3<T>& Chh, Grid3<T>& Che, Grid3<T>& Cee,
                         Grid3<T>& Ceh, const FDTDParams& p) {
	const int Nx = p.whole_region_sizes[0], Ny = p.whole_region_sizes[1],
	          Nz = p.whole_region_sizes[2];
	Chh = Grid3<T>(Nx, Ny, Nz, T(0));
	Che = Grid3<T>(Nx, Ny, Nz, T(0));
	Cee = Grid3<T>(Nx, Ny, Nz, T(0));
	Ceh = Grid3<T>(Nx, Ny, Nz, T(0));

	const CoeffsParams c = MakeCoeffsParams(
	    p.dt, p.eps0, p.eps_fg, p.eps_bg, p.sigma_fg, p.sigma_bg, p.pml_m,
	    p.sigma_max_xy, p.sigma_max_z, p.sim_region, p.opt_region,
	    p.waveguides_areas, p.opt_region_values, p.nxP, p.nyP, p.nzP);

	ChhChe<T>(Chh, Che, 1, Nx - 1, 1, Ny - 1, 1, Nz - 1, c.sim_region_bounds,
	          c.m, c.sigma_max_xy, c.sigma_max_z, p.mu, c.dt, c.eps_bg);
	CeeCeh<T>(Cee, Ceh, 2, Nx, 2, Ny, 2, Nz, c.waveguide_bounds,
	          c.opt_region_bounds, p.opt_region_values, p.nxP, p.nyP, c.eps_fg,
	          c.eps_bg, c.sigma_fg, c.sigma_bg, c.sim_region_bounds, c.m,
	          c.sigma_max_xy, c.sigma_max_z, c.dt);
}

// Recompute coefficients only inside the design box (== UpdateOptRegionCoeffs).
template <class T>
void FillCoeffsOptBox(Grid3<T>& Chh, Grid3<T>& Che, Grid3<T>& Cee,
                      Grid3<T>& Ceh, const FDTDParams& p) {
	const int Nx = p.whole_region_sizes[0], Ny = p.whole_region_sizes[1],
	          Nz = p.whole_region_sizes[2];
	const long xi1 = p.opt_region.r[0][0], xi2 = p.opt_region.r[0][1];
	const long yi1 = p.opt_region.r[1][0], yi2 = p.opt_region.r[1][1];
	const long zi1 = p.opt_region.r[2][0], zi2 = p.opt_region.r[2][1];

	const CoeffsParams c = MakeCoeffsParams(
	    p.dt, p.eps0, p.eps_fg, p.eps_bg, p.sigma_fg, p.sigma_bg, p.pml_m,
	    p.sigma_max_xy, p.sigma_max_z, p.sim_region, p.opt_region,
	    p.waveguides_areas, p.opt_region_values, p.nxP, p.nyP, p.nzP);

	ChhChe<T>(Chh, Che, std::max(1L, xi1), std::min<long>(Nx - 1, xi2),
	          std::max(1L, yi1), std::min<long>(Ny - 1, yi2), std::max(1L, zi1),
	          std::min<long>(Nz - 1, zi2), c.sim_region_bounds, c.m,
	          c.sigma_max_xy, c.sigma_max_z, p.mu, c.dt, c.eps_bg);
	CeeCeh<T>(Cee, Ceh, std::max(2L, xi1), std::min<long>(Nx, xi2),
	          std::max(2L, yi1), std::min<long>(Ny, yi2), std::max(2L, zi1),
	          std::min<long>(Nz, zi2), c.waveguide_bounds, c.opt_region_bounds,
	          p.opt_region_values, p.nxP, p.nyP, c.eps_fg, c.eps_bg, c.sigma_fg,
	          c.sigma_bg, c.sim_region_bounds, c.m, c.sigma_max_xy,
	          c.sigma_max_z, c.dt);
}

}  // namespace lucuma::julia
