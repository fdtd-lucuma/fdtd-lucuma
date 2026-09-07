#!/bin/bash
#SBATCH -J vulkanfdtd
#SBATCH --partition=gpu
#SBATCH --gres=gpu:1
#SBATCH -c 8
#SBATCH --mem=32GB
#SBATCH --time=02:00:00
#SBATCH --output=%x-%j.out
#SBATCH --error=%x-%j.err

set -euo pipefail

module purge > /dev/null 2>&1 || true
for m in gcc/13 cmake ninja eigen glslang vulkan cuda; do
	module load "$m" > /dev/null 2>&1 || true
done

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release .
cmake --build build -j "${SLURM_CPUS_PER_TASK:-8}"

nvidia-smi -L || true

rm -f "input/${LAYOUT}/norm.toml"
/usr/bin/time -v ./build/lucuma-julia "$LAYOUT" --backend vulkan --precision f32
