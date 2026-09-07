#!/bin/bash
#SBATCH -J vulkanfdtd-run
#SBATCH --partition=gpu
#SBATCH --gres=gpu:1
#SBATCH -c 8
#SBATCH --mem=32GB
#SBATCH --time=02:00:00
#SBATCH --output=%x-%j.out
#SBATCH --error=%x-%j.err
# Run optimization.cpp (lucuma-julia) on the GPU. Needs build.sh first.
# LAYOUT=challenge_bend sbatch --export=ALL,LAYOUT run.sh

set -euo pipefail
cd "$(dirname "$0")"
LAYOUT="${LAYOUT:-challenge_bend}"

[ -f .deps/env.sh ] && source .deps/env.sh

rm -f "input/${LAYOUT}/norm.toml"
/usr/bin/time -v ./build/lucuma-julia "$LAYOUT" --backend vulkan --precision f32
