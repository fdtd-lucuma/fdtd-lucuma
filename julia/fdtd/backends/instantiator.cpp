// No direct Julia counterpart — mirrors
// fdtd-lucuma/src/services/backends/instantiator.cpp. SPDX-License-Identifier:
// GPL-3.0-or-later

#include "fdtd/backends/instantiator.hpp"

#include <stdexcept>
#include <type_traits>

#include "fdtd/backends/sequential.hpp"
#include "fdtd/backends/taskflow.hpp"
#include "fdtd/backends/vulkan/vulkan.hpp"
#include "pipeline/4_fdtd_setup.hpp"
#include "pipeline/7_forw_fdtd.hpp"

#include <chrono>
#include <iostream>

namespace lucuma::julia {

ForwardRunOptions MakeForwardRunOptions(const std::filesystem::path &root,
                                        const std::string &layout,
                                        const Debug &debug) {
  ForwardRunOptions opts;
  opts.vis_path = root / "vis" / layout / "fw";
  opts.norm_path = root / "input" / layout / "norm.toml";
  opts.proj = layout;
  opts.calibration_tag = "calibration";
  opts.forward_tag = "1_1_1";
  opts.snapshot_plane = debug.plane;
  opts.snapshot_field = debug.field;
  opts.step = static_cast<int>(debug.step);
  opts.save_result = false;
  return opts;
}

template <class T> std::unique_ptr<IFdtdBackend<T>> MakeBackend(Backend b) {
  switch (b) {
  case Backend::sequential:
    return std::make_unique<Sequential<T>>();
  case Backend::taskflow:
    return std::make_unique<Taskflow<T>>();
  case Backend::vulkan:
    if constexpr (std::is_same_v<T, float>)
      return std::make_unique<Vulkan<float>>();
    else
      throw std::runtime_error(
          "vulkan backend requires --precision f32 (Metal has no double)");
  }
  throw std::runtime_error("unknown backend");
}

template std::unique_ptr<IFdtdBackend<float>> MakeBackend<float>(Backend);
template std::unique_ptr<IFdtdBackend<double>> MakeBackend<double>(Backend);

namespace {
template <class T>
ForwardResult run_typed(Backend b, FDTDParams &params,
                        const std::vector<std::string> &monitors_in,
                        const std::vector<std::string> &monitors_out,
                        const ForwardRunOptions &o) {
  auto be = MakeBackend<T>(b);
  be->init(params);

  auto start = std::chrono::steady_clock::now();

  const SourceNormalization sn = CalibrateIncidentPower<T>(
      params, *be, monitors_in, o.vis_path, o.norm_path, o.calibration_tag,
      o.snapshot_plane, o.snapshot_field);

  const ForwardResult result = RunForwardFDTD<T>(
      params, *be, params.opt_region_values, monitors_in, monitors_out, sn,
      o.vis_path, o.proj, o.forward_tag, o.snapshot_plane, o.snapshot_field,
      o.step, o.save_result);

  auto end = std::chrono::steady_clock::now();

  std::chrono::duration<double> elapsed_seconds = end - start;

  std::cout << "Elapsed time: " << elapsed_seconds.count() << " s\n";

  return result;
}
} // namespace

ForwardResult RunForwardOnBackend(Backend backend, Precision precision,
                                  FDTDParams &params,
                                  const std::vector<std::string> &monitors_in,
                                  const std::vector<std::string> &monitors_out,
                                  const ForwardRunOptions &options) {
  switch (precision) {
  case Precision::f32:
    return run_typed<float>(backend, params, monitors_in, monitors_out,
                            options);
  case Precision::f64:
    return run_typed<double>(backend, params, monitors_in, monitors_out,
                             options);
  }
  throw std::runtime_error("unknown precision");
}

} // namespace lucuma::julia
