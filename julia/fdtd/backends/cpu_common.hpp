// No direct Julia counterpart — mirrors fdtd-lucuma/src/services/backends/cpu_common.cppm.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Bits shared by the Sequential and Taskflow backends: the T-typed running-DFT
// accumulator and the per-step source-injection wrappers.

#pragma once

#include <array>
#include <complex>
#include <map>
#include <utility>
#include <vector>

#include "auxiliaries/array.hpp"
#include "auxiliaries/modes.hpp"  // FreqsField / FreqFields
#include "fdtd/fw_sim.hpp"        // DFTSampleTimes / DFTPhase
#include "fdtd/parallel.hpp"
#include "fdtd/utils.hpp"

namespace lucuma::vulkan {

// Running DFT: per frequency, 6 complex<T> volumes [Ex,Ey,Ez,Hx,Hy,Hz].
template <class T>
struct DftAccum {
	std::vector<double> freqs;
	int Nx = 0, Ny = 0, Nz = 0;
	std::map<double, std::array<Grid3<std::complex<T>>, 6>> acc;

	void init(const std::vector<double>& f, int nx, int ny, int nz) {
		freqs = f;
		Nx = nx;
		Ny = ny;
		Nz = nz;
		acc.clear();
		for (double fr : freqs) {
			std::array<Grid3<std::complex<T>>, 6> a;
			for (auto& g : a)
				g = Grid3<std::complex<T>>(Nx, Ny, Nz, std::complex<T>{});
			acc.emplace(fr, std::move(a));
		}
	}

	void zero() {
		for (auto& [fr, a] : acc)
			for (auto& g : a) g.fill(std::complex<T>{});
	}

	// == AccumulateDFT for one Yee step (default k range = whole grid).
	void accumulate(int t, double dt, const Grid3<T>& Ex, const Grid3<T>& Ey,
	                const Grid3<T>& Ez, const Grid3<T>& Hx, const Grid3<T>& Hy,
	                const Grid3<T>& Hz, int k_lo = 1, int k_hi = -1) {
		if (k_hi < 0) k_hi = Nz;
		const auto [et, mt] = DFTSampleTimes(t, dt);
		for (double fr : freqs) {
			const std::complex<T> pe(DFTPhase(fr, et));
			const std::complex<T> ph(DFTPhase(fr, mt));
			auto& a = acc.at(fr);
			AccumDTF<T>(a[0], a[1], a[2], a[3], a[4], a[5], Ex, Ey, Ez, Hx, Hy,
			            Hz, pe, ph, Nx, Ny, k_lo, k_hi);
		}
	}

	FreqsField toHost() const {
		FreqsField ff;
		for (double fr : freqs) {
			const auto& a = acc.at(fr);
			FreqFields g;
			for (int c = 0; c < 3; ++c) {
				g.E[c] = Grid3<cdouble>(Nx, Ny, Nz, cdouble{});
				g.H[c] = Grid3<cdouble>(Nx, Ny, Nz, cdouble{});
				const auto& se = a[c];
				const auto& sh = a[c + 3];
				for (std::size_t i = 0; i < se.size(); ++i) {
					g.E[c].data()[i] = cdouble(se.data()[i]);
					g.H[c].data()[i] = cdouble(sh.data()[i]);
				}
			}
			ff.emplace(fr, std::move(g));
		}
		return ff;
	}
};

// Cast a T-typed Yee volume to a host double volume (for snapshots).
template <class T>
Grid3<double> CopyGridToHost(const Grid3<T>& g) {
	Grid3<double> out(g.nx(), g.ny(), g.nz());
	for (std::size_t i = 0; i < g.size(); ++i)
		out.data()[i] = static_cast<double>(g.data()[i]);
	return out;
}

// == InjectElectricSources / InjectMagneticSources (fw_sim.jl): compute
// tau / envelope for step t and drive the surface-current kernels.
template <class T>
void InjectElectricSources(Grid3<T>& Ex, Grid3<T>& Ey, Grid3<T>& Ez,
                           const Grid3<T>& Ceh,
                           const std::vector<Source>& sources, int t,
                           double dt) {
	for (const auto& s : sources) {
		const double tau = (static_cast<double>(t) - 0.5) * dt - s.time_origin;
		InjectElectricSurfaceCurrent<T>(Ex, Ey, Ez, Ceh, s, static_cast<T>(tau),
		                                T(1));
	}
}

template <class T>
void InjectMagneticSources(Grid3<T>& Hx, Grid3<T>& Hy, Grid3<T>& Hz,
                           const Grid3<T>& Che,
                           const std::vector<Source>& sources, int t,
                           double dt) {
	for (const auto& s : sources) {
		const double tau = (static_cast<double>(t) - 1.0) * dt - s.time_origin;
		InjectMagneticSurfaceCurrent<T>(Hx, Hy, Hz, Che, s, static_cast<T>(tau),
		                                T(1));
	}
}

}  // namespace lucuma::vulkan
