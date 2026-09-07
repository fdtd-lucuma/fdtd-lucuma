#!/bin/sh
#SBATCH -J vulkanfdtd-run
#SBATCH --partition=gpu
#SBATCH --nodelist=ds001
#SBATCH --gres=gpu:1
#SBATCH -c 4
#SBATCH --mem=32GB
#SBATCH --time=24:00:00
#SBATCH --output=%x-%j.out
#SBATCH --error=%x-%j.err
# Run optimization.cpp (lucuma-julia) on the GPU. Needs build.sh first.
# LAYOUT=challenge_bend sbatch --export=ALL,LAYOUT run.sh

set -eu

cd "${SLURM_SUBMIT_DIR:-$(dirname "$0")}"
LAYOUT="${LAYOUT:-challenge_bend}"

[ -x build/lucuma-julia ] || { echo "build/lucuma-julia missing — run build.sh first"; exit 1; }
[ -f .deps/env.sh ] && . .deps/env.sh

rm -f "input/${LAYOUT}/norm.toml"
time ./build/lucuma-julia "$LAYOUT" --backend vulkan --precision f32
