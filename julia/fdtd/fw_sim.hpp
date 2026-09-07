// == fdtd/fw_sim.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>
#include <complex>
#include <filesystem>
#include <functional>
#include <string>
#include <utility>

#include "auxiliaries/modes.hpp"     // FreqsField
#include "fdtd/backends/i_backend.hpp"
#include "fdtd/params.hpp"
#include "vis/funcs.hpp"

namespace lucuma::vulkan {

// == DFTSampleTimes : (electric_time, magnetic_time) for Yee step t.
std::pair<double, double> DFTSampleTimes(int t, double dt,
                                         double h_offset_steps = -0.5);

// == DFTPhase : exp(-i 2pi f t).
inline std::complex<double> DFTPhase(double frequency_hz, double time_s) {
	return std::exp(
	    std::complex<double>(0.0, -2.0 * M_PI * frequency_hz * time_s));
}

// == FwFDTDSimulation : run the Yee steps with optional snapshots + windowed DFT.
template <class T>
FreqsField FwFDTDSimulation(
    const FDTDParams& p, IFdtdBackend<T>& be,
    const std::filesystem::path& vis_path = {}, const std::string& additional = {},
    const std::string& snapshot_plane = "xz",
    const std::string& snapshot_field = "Ey", int step = 20,
    bool save_result = false, int dft_start_step = 1, int dft_stop_step = -1,
    int dft_window_steps = 0,
    const std::function<void(int, const FreqsField&)>& on_dft_window = {}) {
	be.reset();
	const int T_steps = static_cast<int>(p.t);
	const int dft_stop = dft_stop_step < 0 ? T_steps : dft_stop_step;

	const int comp = snapshot_field == "Ex"   ? 0
	                 : snapshot_field == "Ez" ? 2
	                                          : 1;

	for (int t = 1; t <= T_steps; ++t) {
		const bool in_window = dft_start_step <= t && t <= dft_stop;
		be.step(t, in_window);

		if (in_window && dft_window_steps > 0 &&
		    (t - dft_start_step + 1) % dft_window_steps == 0) {
			if (on_dft_window) on_dft_window(t, be.collectFreqFields());
			if (t < dft_stop) be.resetDftAccumulators();
		}

		if (save_result && step > 0 && t % step == 0)
			FDTDSnapshot(be.collectField(comp), snapshot_field, t, additional,
			             vis_path, snapshot_plane);
	}

	return be.collectFreqFields();
}

}  // namespace lucuma::vulkan
