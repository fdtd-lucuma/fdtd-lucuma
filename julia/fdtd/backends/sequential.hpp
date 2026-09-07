// No direct Julia counterpart — mirrors fdtd-lucuma/src/services/backends/sequential.cppm.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Single-threaded CPU backend: the reference implementation.

#pragma once

#include "auxiliaries/array.hpp"
#include "fdtd/backends/cpu_common.hpp"
#include "fdtd/backends/i_backend.hpp"

namespace lucuma::julia {

template <class T>
class Sequential final : public IFdtdBackend<T> {
public:
	void init(const FDTDParams& p) override;
	void updateOptRegionCoeffs(const FDTDParams& p) override;
	void reset() override;
	void step(int t, bool accumulate_dft = true) override;
	void resetDftAccumulators() override;
	FreqsField collectFreqFields() const override;
	Grid3<double> collectField(int component) const override;

private:
	const FDTDParams* p_ = nullptr;
	Grid3<T> Ex_, Ey_, Ez_, Hx_, Hy_, Hz_;
	Grid3<T> Chh_, Che_, Cee_, Ceh_;
	DftAccum<T> dft_;
};

extern template class Sequential<float>;
extern template class Sequential<double>;

}  // namespace lucuma::julia
