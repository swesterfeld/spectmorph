#!/bin/bash
set -Eeuo pipefail -x

brew install autoconf-archive automake pkg-config libsndfile jack lv2 fftw libao qt5
export PKG_CONFIG_PATH="$(brew --prefix qt@5)/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
./autogen.sh
make -j `sysctl -n hw.ncpu`
make check
