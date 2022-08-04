#!/bin/bash
set -Eeuo pipefail -x

brew install autoconf-archive automake libsndfile jack lv2 fftw
./autogen.sh --without-qt
make
make check
