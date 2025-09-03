#!/usr/bin/env bash

set -ex

source PKGBUILD || true

pacman -Sy
find /var/cache/makepkg/pkg -type f -exec pacman -U --noconfirm {} \+

printf "%s\n" "${depends[@]}" "${makedepends[@]}" "$@" |\
grep -vxFf <( find /var/cache/makepkg/pkg -type f -exec pacman -Qi -p {} \+ | awk '/Name/ {print $3}') |\
xargs -- pacman -Syu --noconfirm --needed
