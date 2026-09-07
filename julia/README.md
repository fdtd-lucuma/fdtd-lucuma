# LucumaFDTD: C++ port of LucumaFDTD

## Requirements

- C++23 compiler (Clang 16+, GCC 13+, Apple Clang 17+), CMake ≥ 3.28, Ninja
- Eigen 3.4 — `apt install libeigen3-dev` / `dnf install eigen3-devel` / `brew install eigen`
- `vulkan` backend only (optional): `glslangValidator` + Vulkan loader/headers, and on macOS
- MoltenVK — `apt install glslang-tools libvulkan-dev` / `brew install glslang vulkan-loader molten-vk`

## Build

```sh
cmake -B build -G Ninja .
cmake --build build
```

## Run

```sh
./build/lucuma-julia [--backend sequential|taskflow|vulkan] [--precision f32|f64]
```

- `--backend` — `sequential`, `taskflow` (default), `vulkan` (GPU; `f32` only).
- `--precision` — `f64` (default) or `f32`.

macOS `vulkan` needs the MoltenVK ICD on the loader path:

```sh
VK_ICD_FILENAMES=$(brew --prefix)/etc/vulkan/icd.d/MoltenVK_icd.json \
  ./build/lucuma-julia --backend vulkan --precision f32
```
