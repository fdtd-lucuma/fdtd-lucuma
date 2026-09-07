// No direct Julia counterpart — mirrors fdtd-lucuma/src/services/backends/vulkan.cppm.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// GPU compute backend (Vulkan). f32 only (single-precision shaders); the instantiator rejects vulkan+f64.

#pragma once

#include <memory>

#include "fdtd/backends/i_backend.hpp"

namespace lucuma::vulkan {

template <class T>
class Vulkan final : public IFdtdBackend<T> {
	static_assert(sizeof(T) == 4,
	              "Vulkan backend is f32 only (the compute shaders are single-precision)");

public:
	Vulkan();
	~Vulkan() override;

	void init(const FDTDParams& p) override;
	void updateOptRegionCoeffs(const FDTDParams& p) override;
	void reset() override;
	void step(int t, bool accumulate_dft = true) override;
	void resetDftAccumulators() override;
	FreqsField collectFreqFields() const override;
	Grid3<double> collectField(int component) const override;

private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};

extern template class Vulkan<float>;

}  // namespace lucuma::vulkan
