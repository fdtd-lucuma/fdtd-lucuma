// == fdtd/utils.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fdtd/utils.hpp"

#include <cmath>

namespace lucuma::julia {

using cplx = std::complex<double>;

// Julia `x^m` for integer m == power_by_squaring.
static double ipow(double b, int e) {
	double r = 1.0, base = b;
	while (e > 0) {
		if (e & 1) r *= base;
		base *= base;
		e >>= 1;
	}
	return r;
}

CoeffsParams MakeCoeffsParams(double dt, double eps0, double eps_fg,
                              double eps_bg, double sigma_fg, double sigma_bg,
                              int m, double sigma_max_xy, double sigma_max_z,
                              const SimRegion& sim_region,
                              const BBox& opt_region,
                              const std::vector<BBox>& waveguide_areas,
                              const std::vector<double>& opt_region_values,
                              int nxP, int nyP, int nzP) {
	CoeffsParams c;
	c.dt = dt;
	c.eps_fg = eps_fg * eps0;
	c.eps_bg = eps_bg * eps0;
	c.sigma_fg = sigma_fg;
	c.sigma_bg = sigma_bg;
	c.m = m;
	c.sigma_max_xy = sigma_max_xy;
	c.sigma_max_z = sigma_max_z;
	c.sim_region_bounds = BoundsSimRegion(sim_region);
	c.opt_region_bounds = BoundsRegion(opt_region);
	for (const auto& wg : waveguide_areas)
		c.waveguide_bounds.push_back(BoundsRegion(wg));
	c.opt_region_values = &opt_region_values;
	c.nxP = nxP;
	c.nyP = nyP;
	c.nzP = nzP;
	return c;
}

double GetSigmaPML(long i, long j, long k, const Bounds& s, int m,
                   double sigma_max_xy, double sigma_max_z) {
	const long x1 = s.x_min, x2 = s.x_max;
	const long y1 = s.y_min, y2 = s.y_max;
	const long z1 = s.z_min, z2 = s.z_max;
	const double n_xy = static_cast<double>(x1 - 1);
	const double n_z = static_cast<double>(z1 - 1);

	double v = 0.0;
	if (i < x1) v += sigma_max_xy * ipow((x1 - i) / n_xy, m);
	if (i > x2) v += sigma_max_xy * ipow((i - x2) / n_xy, m);
	if (j < y1) v += sigma_max_xy * ipow((y1 - j) / n_xy, m);
	if (j > y2) v += sigma_max_xy * ipow((j - y2) / n_xy, m);
	if (k < z1) v += sigma_max_z * ipow((z1 - k) / n_z, m);
	if (k > z2) v += sigma_max_z * ipow((k - z2) / n_z, m);
	return v;
}

double GetEps(long i, long j, long k, const std::vector<Bounds>& wg,
              const Bounds& opt, const std::vector<double>& P, int nxP, int nyP,
              double eps_fg, double eps_bg) {
	for (const auto& b : wg)
		if (b.x_min <= i && i <= b.x_max && b.y_min <= j && j <= b.y_max &&
		    b.z_min <= k && k <= b.z_max)
			return eps_fg;

	if (opt.x_min <= i && i <= opt.x_max && opt.y_min <= j && j <= opt.y_max &&
	    opt.z_min <= k && k <= opt.z_max) {
		const long ox = i - opt.x_min + 1;
		const long oy = j - opt.y_min + 1;
		const long oz = k - opt.z_min + 1;
		const double p =
		    P[PIndex(static_cast<int>(ox - 1), static_cast<int>(oy - 1),
		             static_cast<int>(oz - 1), nxP, nyP)];
		return eps_bg + p * (eps_fg - eps_bg);
	}
	return eps_bg;
}

double GetSigma(long i, long j, long k, const std::vector<Bounds>& wg,
                const Bounds& opt, const std::vector<double>& P, int nxP,
                int nyP, double sigma_fg, double sigma_bg) {
	for (const auto& b : wg)
		if (b.x_min <= i && i <= b.x_max && b.y_min <= j && j <= b.y_max &&
		    b.z_min <= k && k <= b.z_max)
			return sigma_fg;

	if (opt.x_min <= i && i <= opt.x_max && opt.y_min <= j && j <= opt.y_max &&
	    opt.z_min <= k && k <= opt.z_max) {
		const long ox = i - opt.x_min + 1;
		const long oy = j - opt.y_min + 1;
		const long oz = k - opt.z_min + 1;
		const double p =
		    P[PIndex(static_cast<int>(ox - 1), static_cast<int>(oy - 1),
		             static_cast<int>(oz - 1), nxP, nyP)];
		return sigma_bg + p * (sigma_fg - sigma_bg);
	}
	return sigma_bg;
}

// ---------------------------------------------------------------------------
// CreateSources
// ---------------------------------------------------------------------------

static Source make_source(const EquivalentModalSource& src) {
	const int size1 = static_cast<int>(src.range1.size());
	const int size2 = static_cast<int>(src.range2.size());
	const int nfreqs = static_cast<int>(src.freqs.size());
	const std::size_t n = static_cast<std::size_t>(size1) * size2 * nfreqs;

	Source s;
	s.Jx.assign(n, cplx{}); s.Jy.assign(n, cplx{}); s.Jz.assign(n, cplx{});
	s.Mx.assign(n, cplx{}); s.My.assign(n, cplx{}); s.Mz.assign(n, cplx{});

	for (int nf = 0; nf < nfreqs; ++nf) {
		const auto& mode = src.mode_by_freq.at(src.freqs[nf]);
		const SurfaceCurrents sc = EquivalentSurfaceCurrents(mode, src.section);
		const cplx amp = src.amplitudes[nf];
		for (int b = 1; b <= size2; ++b)
			for (int a = 1; a <= size1; ++a) {
				const std::size_t idx =
				    static_cast<std::size_t>(a - 1) +
				    static_cast<std::size_t>(size1) *
				        ((b - 1) + static_cast<std::size_t>(size2) * nf);
				s.Jx[idx] = amp * sc.J[0](a - 1, b - 1);
				s.Jy[idx] = amp * sc.J[1](a - 1, b - 1);
				s.Jz[idx] = amp * sc.J[2](a - 1, b - 1);
				s.Mx[idx] = amp * sc.M[0](a - 1, b - 1);
				s.My[idx] = amp * sc.M[1](a - 1, b - 1);
				s.Mz[idx] = amp * sc.M[2](a - 1, b - 1);
			}
	}

	s.omegas.resize(nfreqs);
	for (int nf = 0; nf < nfreqs; ++nf) s.omegas[nf] = 2.0 * M_PI * src.freqs[nf];

	s.nfreqs = nfreqs;
	s.axis = src.axis;
	s.electric_fixed_index = src.fixed_index;
	s.magnetic_fixed_index = MagneticSheetIndex(src.section);
	s.r1_lo = src.range1.first();
	s.r2_lo = src.range2.first();
	s.size1 = size1;
	s.size2 = size2;
	s.inv_delta_normal = 1.0 / src.section.delta_normal;
	s.J0 = src.J0;
	s.time_origin = src.time_pulse.origin();
	return s;
}

std::vector<Source> CreateSources(
    const std::vector<EquivalentModalSource>& sources) {
	std::vector<Source> out;
	out.reserve(sources.size());
	for (const auto& s : sources) out.push_back(make_source(s));
	return out;
}

}  // namespace lucuma::julia
