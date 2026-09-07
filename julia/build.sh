#!/bin/bash
#SBATCH -J vulkanfdtd-build
#SBATCH -c 8
#SBATCH --mem=16GB
#SBATCH --time=00:40:00
#SBATCH --output=%x-%j.out
#SBATCH --error=%x-%j.err
# Build the C++ Vulkan port. Run on a login node or: sbatch build.sh

set -euo pipefail

JULIA_DIR="$(cd "$(dirname "$0")" && pwd)"
PREFIX="$JULIA_DIR/.deps"
JOBS="${SLURM_CPUS_PER_TASK:-8}"
mkdir -p "$PREFIX"

module purge > /dev/null 2>&1 || true
for m in gnu14 gnu13 gnu12 gcc; do module load "$m" > /dev/null 2>&1 && break || true; done
g++ --version | head -1

# cmake + ninja from a venv (the module cmake is broken; pip wheels are self-contained)
python3 -m venv "$PREFIX/venv"
source "$PREFIX/venv/bin/activate"
pip install --quiet --upgrade pip cmake ninja
cmake --version | head -1

# Vulkan headers + loader — build from source only if the node lacks them
have_vulkan() { echo '#include <vulkan/vulkan.h>' | g++ -x c++ -E - > /dev/null 2>&1 \
                && ldconfig -p 2>/dev/null | grep -q 'libvulkan\.so\.1'; }
VK_ARGS=()
RUNTIME_LDPATH=""
if have_vulkan; then
	echo "system Vulkan loader/headers OK"
else
	echo "building Vulkan-Headers + Vulkan-Loader into $PREFIX"
	mkdir -p "$PREFIX/src" && cd "$PREFIX/src"
	for repo in Vulkan-Headers Vulkan-Loader; do
		[ -d "$repo" ] || git clone --depth 1 "https://github.com/KhronosGroup/${repo}.git"
		# Headless compute node: no window-system integration (avoids X11/xrandr/wayland deps)
		cmake -S "$repo" -B "$repo/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_PREFIX_PATH="$PREFIX" \
			-DUPDATE_DEPS=OFF \
			-DBUILD_WSI_XCB_SUPPORT=OFF -DBUILD_WSI_XLIB_SUPPORT=OFF \
			-DBUILD_WSI_WAYLAND_SUPPORT=OFF -DBUILD_WSI_DIRECTFB_SUPPORT=OFF \
			-DBUILD_WSI_SCREEN_QNX_SUPPORT=OFF
		cmake --build "$repo/build" -j "$JOBS"
		cmake --install "$repo/build"
	done
	VK_ARGS=(-DCMAKE_PREFIX_PATH="$PREFIX")
	RUNTIME_LDPATH="$PREFIX/lib:$PREFIX/lib64"
fi

# Build (no glslangValidator on the cluster: uses shaders/prebuilt/*.h)
cd "$JULIA_DIR"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DGLSLANG=GLSLANG-NOTFOUND "${VK_ARGS[@]}"
cmake --build build -j "$JOBS"

# Runtime env for run.sh
: > "$PREFIX/env.sh"
[ -n "$RUNTIME_LDPATH" ] && echo "export LD_LIBRARY_PATH=\"$RUNTIME_LDPATH:\${LD_LIBRARY_PATH:-}\"" >> "$PREFIX/env.sh"
echo "build OK -> $JULIA_DIR/build/lucuma-julia"
