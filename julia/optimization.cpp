// vulkan-fdtd [--root DIR] [--backend sequential|taskflow|vulkan] [--precision
// f32|f64]

#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "auxiliaries/funcs.hpp"
#include "auxiliaries/modes.hpp"
#include "fdtd/backends/instantiator.hpp"
#include "fdtd/params.hpp"
#include "pipeline/0_loads.hpp"
#include "pipeline/1_params.hpp"
#include "pipeline/2_device.hpp"
#include "pipeline/3_wg_source.hpp"
#include "pipeline/4_fdtd_setup.hpp"
#include "pipeline/7_forw_fdtd.hpp"
#include "utils/precision.hpp"

#ifndef VULKAN_FDTD_ROOT
#define VULKAN_FDTD_ROOT "."
#endif

namespace fs = std::filesystem;

namespace lucuma::vulkan {

ForwardResult Optimization(const std::string &layout, const fs::path &root,
                           Backend backend, Precision precision) {

  const fs::path input_dir = root / "input" / layout;
  const fs::path output_dir = root / "output" / layout;
  const fs::path params_path = input_dir / "params.toml";
  const fs::path device_path = input_dir / "device.toml";
  const fs::path optimizer_path = input_dir / "optimizer.toml";
  const fs::path debug_path = input_dir / "debug.toml";
  const fs::path p_path = output_dir / "P.csv";

  LoadInputs(root / "input", layout);
  const Stages stages = StagesParams(params_path);
  const Elements elements = ElementsParams(device_path);
  const Filters _ = FiltersParams(params_path, stages.stage);
  const Optimizer optimizer = OptimizerParams(params_path, optimizer_path);
  const Debug debug = DebugParams(debug_path);

  if (stages.stage == "cont") {
    DefineParams(params_path);
    DefineDevice(params_path, device_path, p_path, true);
  }

  const Params params = LoadParams(params_path);
  const Params device = LoadParams(device_path);

  const Grid3<double> eps_grid =
      BuildPermGrid(params_path, device_path, p_path);

  const auto waveguide_names =
      unite(elements.waveguides_in, elements.waveguides_out);
  const auto monitor_names = unite(elements.monitors_in, elements.monitors_out);

  std::map<std::string, ModeMonitor> monitors;
  for (const auto &mn : monitor_names)
    monitors[mn] = DefineModeMonitor(params, device, mn, eps_grid, 8, 4);

  const auto modal_sources =
      DefineModalSources(params, device, elements.sources, eps_grid, 8, 1.0, 4);

  FDTDParams fp =
      FDTDSetUp(params_path, device_path, p_path, modal_sources,
                elements.sources, waveguide_names, monitors, monitor_names);

  const ForwardRunOptions opts = MakeForwardRunOptions(root, layout, debug);

  return RunForwardOnBackend(backend, precision, fp, elements.monitors_in,
                             elements.monitors_out, opts);
}

} // namespace lucuma::vulkan

int main(int argc, char **argv) {
  using namespace lucuma::vulkan;

  std::string layout = "challenge_bend";
  std::filesystem::path root = VULKAN_FDTD_ROOT;
  Backend backend = Backend::taskflow;
  Precision precision = Precision::f64;

  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "--root" && i + 1 < argc) {
      root = argv[++i];
    } else if (a == "--backend" && i + 1 < argc) {
      std::string v = argv[++i];
      if (v == "sequential")
        backend = Backend::sequential;
      else if (v == "taskflow")
        backend = Backend::taskflow;
      else if (v == "vulkan")
        backend = Backend::vulkan;
      else {
        std::cerr << "unknown --backend " << v << "\n";
        return 2;
      }
    } else if (a == "--precision" && i + 1 < argc) {
      std::string v = argv[++i];
      if (v == "f32")
        precision = Precision::f32;
      else if (v == "f64")
        precision = Precision::f64;
      else {
        std::cerr << "unknown --precision " << v << "\n";
        return 2;
      }
    } else if (!a.empty() && a[0] != '-') {
      layout = a;
    }
  }

  try {
    const ForwardResult result = Optimization(layout, root, backend, precision);
    std::cout << result.loss << "\n" << result.transmittance;
    return EXIT_SUCCESS;
  } catch (const std::exception &e) {
    std::cerr << "error: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
