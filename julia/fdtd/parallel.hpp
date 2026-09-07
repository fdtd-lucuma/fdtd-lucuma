// == fdtd/parallel.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>
#include <complex>
#include <cstddef>

#include "auxiliaries/array.hpp"
#include "fdtd/utils.hpp"

namespace lucuma::vulkan {

// H update over i in 1..i_hi (== Nx-1), j in 1..j_hi, k in k_lo..k_hi
// (k_lo lets the Taskflow backend partition the k axis; == 1 for the whole run).
template <class T>
void UpdateH(Grid3<T>& Hx, Grid3<T>& Hy, Grid3<T>& Hz, const Grid3<T>& Ex,
            const Grid3<T>& Ey, const Grid3<T>& Ez, const Grid3<T>& Chh,
            const Grid3<T>& Che, long i_hi, long j_hi, long k_lo, long k_hi,
            T dx, T dy, T dz) {
	for (long k = k_lo; k <= k_hi; ++k)
		for (long j = 1; j <= j_hi; ++j)
			for (long i = 1; i <= i_hi; ++i) {
				const int I = int(i), J = int(j), K = int(k);
				const T chh = Chh(I, J, K), che = Che(I, J, K);
				Hx(I, J, K) = chh * Hx(I, J, K) +
				              che * ((Ey(I, J, K + 1) - Ey(I, J, K)) / dz -
				                     (Ez(I, J + 1, K) - Ez(I, J, K)) / dy);
				Hy(I, J, K) = chh * Hy(I, J, K) +
				              che * ((Ez(I + 1, J, K) - Ez(I, J, K)) / dx -
				                     (Ex(I, J, K + 1) - Ex(I, J, K)) / dz);
				Hz(I, J, K) = chh * Hz(I, J, K) +
				              che * ((Ex(I, J + 1, K) - Ex(I, J, K)) / dy -
				                     (Ey(I + 1, J, K) - Ey(I, J, K)) / dx);
			}
}

// E update over i in 2..i_hi (== Nx), j in 2..j_hi, k in k_lo..k_hi (k_lo == 2
// for the whole run; partitioned by the Taskflow backend).
template <class T>
void UpdateE(Grid3<T>& Ex, Grid3<T>& Ey, Grid3<T>& Ez, const Grid3<T>& Hx,
            const Grid3<T>& Hy, const Grid3<T>& Hz, const Grid3<T>& Cee,
            const Grid3<T>& Ceh, long i_hi, long j_hi, long k_lo, long k_hi,
            T dx, T dy, T dz) {
	for (long k = k_lo; k <= k_hi; ++k)
		for (long j = 2; j <= j_hi; ++j)
			for (long i = 2; i <= i_hi; ++i) {
				const int I = int(i), J = int(j), K = int(k);
				const T cee = Cee(I, J, K), ceh = Ceh(I, J, K);
				Ex(I, J, K) = cee * Ex(I, J, K) +
				              ceh * ((Hz(I, J, K) - Hz(I, J - 1, K)) / dy -
				                     (Hy(I, J, K) - Hy(I, J, K - 1)) / dz);
				Ey(I, J, K) = cee * Ey(I, J, K) +
				              ceh * ((Hx(I, J, K) - Hx(I, J, K - 1)) / dz -
				                     (Hz(I, J, K) - Hz(I - 1, J, K)) / dx);
				Ez(I, J, K) = cee * Ez(I, J, K) +
				              ceh * ((Hy(I, J, K) - Hy(I - 1, J, K)) / dx -
				                     (Hx(I, J, K) - Hx(I, J - 1, K)) / dy);
			}
}

// Running-DFT accumulation over i in 1..Nx, j in 1..Ny, k in k_lo..k_hi.
template <class T>
void AccumDTF(Grid3<std::complex<T>>& E1, Grid3<std::complex<T>>& E2,
             Grid3<std::complex<T>>& E3, Grid3<std::complex<T>>& H1,
             Grid3<std::complex<T>>& H2, Grid3<std::complex<T>>& H3,
             const Grid3<T>& Ex, const Grid3<T>& Ey, const Grid3<T>& Ez,
             const Grid3<T>& Hx, const Grid3<T>& Hy, const Grid3<T>& Hz,
             std::complex<T> phase_E, std::complex<T> phase_H, int Nx, int Ny,
             int k_lo, int k_hi) {
	for (int k = k_lo; k <= k_hi; ++k)
		for (int j = 1; j <= Ny; ++j)
			for (int i = 1; i <= Nx; ++i) {
				E1(i, j, k) += Ex(i, j, k) * phase_E;
				E2(i, j, k) += Ey(i, j, k) * phase_E;
				E3(i, j, k) += Ez(i, j, k) * phase_E;
				H1(i, j, k) += Hx(i, j, k) * phase_H;
				H2(i, j, k) += Hy(i, j, k) * phase_H;
				H3(i, j, k) += Hz(i, j, k) * phase_H;
			}
}

// Shared body of the two surface-current kernels. `cur` are the 3 current
// components (column-major size1*size2*nfreqs, complex<double>), `fixed` the
// sheet index along the port normal.
template <class T>
inline void inject_surface_current(
    Grid3<T>& Fx, Grid3<T>& Fy, Grid3<T>& Fz, const Grid3<T>& coef,
    const Source& s, long fixed,
    const std::array<const std::vector<std::complex<double>>*, 3>& cur, T tau,
    T envelope) {
	const T scale = T(s.J0) * envelope * T(s.inv_delta_normal);

	for (int b = 1; b <= s.size2; ++b)
		for (int a = 1; a <= s.size1; ++a) {
			long i = fixed, j = fixed, k = fixed;
			if (s.axis == Axis::Z) {
				i = s.r1_lo + a - 1;
				j = s.r2_lo + b - 1;
			} else if (s.axis == Axis::X) {
				j = s.r1_lo + a - 1;
				k = s.r2_lo + b - 1;
			} else {
				i = s.r1_lo + a - 1;
				k = s.r2_lo + b - 1;
			}

			T sx = 0, sy = 0, sz = 0;
			for (int n = 0; n < s.nfreqs; ++n) {
				const T theta = T(s.omegas[n]) * tau;
				const T c = std::cos(theta), sn = std::sin(theta);
				const std::size_t idx =
				    std::size_t(a - 1) +
				    std::size_t(s.size1) * ((b - 1) + std::size_t(s.size2) * n);
				const auto cx = (*cur[0])[idx];
				const auto cy = (*cur[1])[idx];
				const auto cz = (*cur[2])[idx];
				sx += T(cx.real()) * c - T(cx.imag()) * sn;
				sy += T(cy.real()) * c - T(cy.imag()) * sn;
				sz += T(cz.real()) * c - T(cz.imag()) * sn;
			}

			const int I = int(i), J = int(j), K = int(k);
			const T g = coef(I, J, K) * scale;
			Fx(I, J, K) -= g * sx;
			Fy(I, J, K) -= g * sy;
			Fz(I, J, K) -= g * sz;
		}
}

template <class T>
void InjectElectricSurfaceCurrent(Grid3<T>& Ex, Grid3<T>& Ey, Grid3<T>& Ez,
                                  const Grid3<T>& Ceh, const Source& s, T tau,
                                  T envelope) {
	inject_surface_current<T>(Ex, Ey, Ez, Ceh, s, s.electric_fixed_index,
	                          {&s.Jx, &s.Jy, &s.Jz}, tau, envelope);
}

template <class T>
void InjectMagneticSurfaceCurrent(Grid3<T>& Hx, Grid3<T>& Hy, Grid3<T>& Hz,
                                  const Grid3<T>& Che, const Source& s, T tau,
                                  T envelope) {
	inject_surface_current<T>(Hx, Hy, Hz, Che, s, s.magnetic_fixed_index,
	                          {&s.Mx, &s.My, &s.Mz}, tau, envelope);
}

}  // namespace lucuma::vulkan
