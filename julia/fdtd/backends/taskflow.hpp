// No direct Julia counterpart — mirrors fdtd-lucuma/src/services/backends/cpu_taskflow.cppm.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Multi-threaded CPU backend: the same kernels as Sequential, with the Yee /
// DFT sweeps partitioned along k across a tf::Executor.

#pragma once

#include <memory>

#include "auxiliaries/array.hpp"
#include "fdtd/backends/cpu_common.hpp"
#include "fdtd/backends/i_backend.hpp"

namespace tf {
class Executor;
}

namespace lucuma::vulkan {

template <class T>
class Taskflow final : public IFdtdBackend<T> {
public:
	Taskflow();
	~Taskflow() override;

	void init(const FDTDParams& p) override;
	void updateOptRegionCoeffs(const FDTDParams& p) override;
	void reset() override;
	void step(int t, bool accumulate_dft = true) override;
	void resetDftAccumulators() override;
	FreqsField collectFreqFields() const override;
	Grid3<double> collectField(int component) const override;

private:
	std::unique_ptr<tf::Executor> exec_;
	const FDTDParams* p_ = nullptr;
	Grid3<T> Ex_, Ey_, Ez_, Hx_, Hy_, Hz_;
	Grid3<T> Chh_, Che_, Cee_, Ceh_;
	DftAccum<T> dft_;
};

extern template class Taskflow<float>;
extern template class Taskflow<double>;

}  // namespace lucuma::vulkan
