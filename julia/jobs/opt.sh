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
# Build + run. Needs ./init.sh first. Submit from julia/jobs/ :
#   cd julia/jobs && sbatch opt.sh
# LAYOUT=challenge_bend sbatch --export=ALL,LAYOUT opt.sh

set -eu

cd "${SLURM_SUBMIT_DIR:-$(dirname "$0")}/.."   # -> julia/
LAYOUT="${LAYOUT:-challenge_bend}"
JOBS="${SLURM_CPUS_PER_TASK:-8}"

[ -f .deps/env.sh ] || { echo ".deps/env.sh missing — run ./init.sh first"; exit 1; }
. .deps/env.sh

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release ${VULKAN_CMAKE_ARGS}
cmake --build build -j "$JOBS"

rm -f "input/${LAYOUT}/norm.toml"
time ./build/vulkan-fdtd "$LAYOUT" --backend vulkan --precision f32
