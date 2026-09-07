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

set -euo pipefail
cd "$(dirname "$0")"
LAYOUT="${LAYOUT:-challenge_bend}"

[ -f .deps/env.sh ] && source .deps/env.sh

rm -f "input/${LAYOUT}/norm.toml"
time ./build/lucuma-julia "$LAYOUT" --backend vulkan --precision f32
