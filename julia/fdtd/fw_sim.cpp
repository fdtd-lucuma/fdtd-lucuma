// == fdtd/fw_sim.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fdtd/fw_sim.hpp"

namespace lucuma::vulkan {

std::pair<double, double> DFTSampleTimes(int t, double dt,
                                         double h_offset_steps) {
	return {static_cast<double>(t) * dt,
	        (static_cast<double>(t) + h_offset_steps) * dt};
}

}  // namespace lucuma::vulkan
