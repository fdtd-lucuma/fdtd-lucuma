[![C/C++ CI](https://github.com/fdtd-lucuma/fdtd-vulkan/workflows/C/C++%20CI/badge.svg)](https://github.com/fdtd-lucuma/fdtd-vulkan/actions?query=workflow%3A%22C%2FC%2B%2B+CI%22)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/fdtd-lucuma/fdtd-vulkan?logo=github)
![GitHub](https://img.shields.io/github/license/fdtd-lucuma/fdtd-vulkan?logo=gnu)

# Una GUI para fdtd

## Dependencies

Read [./pkg/ubuntu/dependencies.txt](./pkg/ubuntu/dependencies.txt)

## Build (Ubuntu>=25.04) Broken
``` bash
git clone https://github.com/fdtd-lucuma/fdtd-vulkan
cd fdtd-vulkan

xargs -oa pkg/ubuntu/dependencies.txt -- sudo apt install

cmake -B build -G Ninja
cmake --build build

cd build
./fdtd-vulkan
```

## Build (Arch Linux)
``` bash
git clone https://github.com/fdtd-lucuma/fdtd-vulkan
cd fdtd-vulkan

paru -Bi pkg/arch/

fdtd-vulkan
```
