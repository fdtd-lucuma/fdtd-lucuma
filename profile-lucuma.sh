#!/bin/bash

#SBATCH --job-name=fdtd-lucuma
#SBATCH --partition=gpu
#SBATCH --gpus=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=16000
#SBATCH --nodelist=ag001

OUT_DIR=~/profile/fdtd-lucuma

mkdir -p "$OUT_DIR"

# Warmup
fdtd-lucuma -x 64 -y 64 -z 64 -b vulkan -t 300

seq 16 16 640 | xargs -I{} junest nsys profile -o "$OUT_DIR/{}" -t vulkan --gpu-metrics-devices=cuda-visible --sample=none fdtd-lucuma -x {} -y {} -z {} -b vulkan -t 300 "$@"
