#!/usr/bin/env bash

set -ex

source PKGBUILD || true

paru \
	-Syu \
	--batchinstall \
	--skipreview \
	--noconfirm \
	--nocleanafter \
	--needed \
	"${depends[@]}" "${makedepends[@]}"
