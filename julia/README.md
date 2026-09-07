# VulkanFDTD: C++ Port of LucumaFDTD

## Requirements

- C++23 compiler (GCC 13+, Clang 16+, Apple Clang 17+)
- CMake ≥ 3.28, Ninja
- Eigen 3.4 — `apt install libeigen3-dev` / `dnf install eigen3-devel` / `brew install eigen`
- `vulkan` backend (optional): `glslangValidator` + Vulkan loader/headers;
  on macOS also MoltenVK — `brew install glslang vulkan-loader molten-vk`

Compute shaders are also committed as prebuilt SPIR-V (`fdtd/backends/vulkan/shaders/prebuilt/`),
so the build works without `glslangValidator` (`-DGLSLANG=GLSLANG-NOTFOUND`).

```sh
sh init.sh
```

## Build

```sh
cmake -B build -G Ninja .
cmake --build build
```

## Run

```sh
./build/vulkan-fdtd [layout] [--backend sequential|taskflow|vulkan] [--precision f32|f64]
```

- `layout` — input set under `input/<layout>/` (default `challenge_bend`)
- `--backend` — `sequential`, `taskflow` (default), `vulkan` (GPU; `f32` only)
- `--precision` — `f64` (default) or `f32`

macOS `vulkan` needs the MoltenVK ICD on the loader path:

```sh
VK_ICD_FILENAMES=$(brew --prefix)/etc/vulkan/icd.d/MoltenVK_icd.json \
  ./build/vulkan-fdtd --backend vulkan --precision f32
```

## Challenges

```sh
./build/vulkan-bend-test
./build/vulkan-wdm-test
./build/vulkan-mode_converter-test
./build/vulkan-beam_splitter-test
```
Override either side:

```sh
./build/vulkan-wdm-test challenges/wdm           
./build/vulkan-wdm-test --out ~/runs/wdm
./build/vulkan-wdm-test ~/designs/wdm --out ~/runs/wdm
```

`--backend` / `--precision` as above.
