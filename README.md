[![C/C++ CI](https://github.com/fdtd-lucuma/fdtd-vulkan/workflows/C/C++%20CI/badge.svg)](https://github.com/fdtd-lucuma/fdtd-vulkan/actions?query=workflow%3A%22C%2FC%2B%2B+CI%22)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/fdtd-lucuma/fdtd-vulkan?logo=github)
![GitHub](https://img.shields.io/github/license/fdtd-lucuma/fdtd-vulkan?logo=gnu)

# Una GUI para fdtd

## Now with a pretty fast compilation (thanks to C++20 modules)
<img width="1366" height="768" alt="imagen" src="https://github.com/user-attachments/assets/f2020b2c-6a15-4262-b4d8-fc24c8a6fcbc" />


## Run

```bash
git clone https://github.com/fdtd-lucuma/fdtd-vulkan
cd fdtd-vulkan

docker compose up
```

## Dependencies

Read [./pkg/ubuntu/dependencies.txt](./pkg/ubuntu/dependencies.txt)

## Build (Debian)
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
