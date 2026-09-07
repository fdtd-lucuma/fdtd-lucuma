// No direct Julia counterpart — mirrors fdtd-lucuma/src/services/backends/i_backend.cppm.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "auxiliaries/modes.hpp"   // FreqsField
#include "fdtd/params.hpp"

namespace lucuma::vulkan {

// A forward-FDTD compute backend, templated on the Yee-field scalar type
// (T = float | double). The mode solver, monitors and modal-output reductions
// stay on the host in double; only the time-domain kernels vary by backend.
template <class T>
class IFdtdBackend {
public:
	virtual ~IFdtdBackend() = default;

	// One-time: allocate field / coefficient / DFT / source resources sized to
	// `p`, upload the design density + source sheets, fill the coefficients over
	// the whole grid.
	virtual void init(const FDTDParams& p) = 0;

	// Refill the coefficients inside the design box after opt_region_values
	// changed (== UpdateOptRegionCoeffs).
	virtual void updateOptRegionCoeffs(const FDTDParams& p) = 0;

	// Zero the Yee fields and the running-DFT accumulators for a fresh run.
	virtual void reset() = 0;

	// UpdateH; inject M; UpdateE; inject E; fold into the DFT when accumulate_dft.
	virtual void step(int t, bool accumulate_dft = true) = 0;

	// Zero only the running-DFT accumulators (windowed DFT).
	virtual void resetDftAccumulators() = 0;

	// Accumulated per-frequency DFT volumes -> host complex<double>.
	virtual FreqsField collectFreqFields() const = 0;

	// One live Yee field volume -> host double. component 0..5 = Ex,Ey,Ez,Hx,Hy,Hz.
	virtual Grid3<double> collectField(int component) const = 0;
};

}  // namespace lucuma::vulkan
