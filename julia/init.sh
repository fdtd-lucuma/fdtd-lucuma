#!/bin/sh
#SBATCH -J vulkanfdtd-opt
#SBATCH --partition=gpu
#SBATCH --nodelist=ds001
#SBATCH --gres=gpu:1
#SBATCH -c 4
#SBATCH --mem=32GB
#SBATCH --time=24:00:00
#SBATCH --output=%x-%j.out
#SBATCH --error=%x-%j.err

set -euo pipefail

JULIA_DIR="$(cd "${SLURM_SUBMIT_DIR:-$(dirname "$0")}" && pwd)"
PREFIX="$JULIA_DIR/.deps"
JOBS="${SLURM_CPUS_PER_TASK:-8}"
mkdir -p "$PREFIX"

# ---- compiler ----
module purge > /dev/null 2>&1 || true
GNU_MOD=""
for m in gnu14 gnu13 gnu12 gcc; do
	module load "$m" > /dev/null 2>&1 && GNU_MOD="$m" && break || true
done
g++ --version | head -1

# ---- cmake + ninja (module cmake is broken; pip wheels are self-contained) ----
python3 -m venv "$PREFIX/venv"
"$PREFIX/venv/bin/pip" install --quiet --upgrade pip cmake ninja
export PATH="$PREFIX/venv/bin:$PATH"
cmake --version | head -1

# ---- Vulkan headers + loader (only if the node lacks them) ----
have_vulkan() { echo '#include <vulkan/vulkan.h>' | g++ -x c++ -E - > /dev/null 2>&1 \
                && ldconfig -p 2>/dev/null | grep -q 'libvulkan\.so\.1'; }
VK_ARG=""
RUNTIME_LDPATH=""
if have_vulkan; then
	echo "system Vulkan loader/headers OK"
else
	echo "building Vulkan-Headers + Vulkan-Loader into $PREFIX"
	mkdir -p "$PREFIX/src" && cd "$PREFIX/src"
	for repo in Vulkan-Headers Vulkan-Loader; do
		[ -d "$repo" ] || git clone --depth 1 "https://github.com/KhronosGroup/${repo}.git"
		# headless compute node: no window-system integration
		cmake -S "$repo" -B "$repo/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_PREFIX_PATH="$PREFIX" \
			-DUPDATE_DEPS=OFF \
			-DBUILD_WSI_XCB_SUPPORT=OFF -DBUILD_WSI_XLIB_SUPPORT=OFF \
			-DBUILD_WSI_WAYLAND_SUPPORT=OFF -DBUILD_WSI_DIRECTFB_SUPPORT=OFF \
			-DBUILD_WSI_SCREEN_QNX_SUPPORT=OFF
		cmake --build "$repo/build" -j "$JOBS"
		cmake --install "$repo/build"
	done
	VK_ARG="-DCMAKE_PREFIX_PATH=$PREFIX"
	RUNTIME_LDPATH="$PREFIX/lib:$PREFIX/lib64"
fi

# ---- Eigen (header-only) ----
if [ ! -d "$PREFIX/eigen/Eigen" ]; then
	git clone --depth 1 https://gitlab.com/libeigen/eigen.git "$PREFIX/eigen" \
		|| git clone --depth 1 https://github.com/eigen-mirror/eigen.git "$PREFIX/eigen"
fi
EIGEN_ARG="-DEIGEN3_INCLUDE_DIR=$PREFIX/eigen"

# ---- record env for jobs/opt.sh ----
{
	echo "command -v module >/dev/null 2>&1 && module load ${GNU_MOD:-gcc} >/dev/null 2>&1 || true"
	echo "export PATH=\"$PREFIX/venv/bin:\$PATH\""
	[ -n "$RUNTIME_LDPATH" ] && echo "export LD_LIBRARY_PATH=\"$RUNTIME_LDPATH:\${LD_LIBRARY_PATH:-}\""
	echo "export VULKAN_CMAKE_ARGS=\"-DGLSLANG=GLSLANG-NOTFOUND $VK_ARG $EIGEN_ARG\""
} > "$PREFIX/env.sh"
