#!/bin/bash
set -Eeo pipefail -x

brew install autoconf-archive automake pkg-config libsndfile jack lv2 fftw libao qt5 scipy libtool
export PKG_CONFIG_PATH="$(brew --prefix qt@5)/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
export PATH="$(brew --prefix python)/bin:$PATH"
export UBSAN_OPTIONS=halt_on_error=1
export NPROC=$(sysctl -n hw.ncpu)

if [[ "$1" == "sanitize" ]]; then
  ./autogen.sh --with-download-instruments --enable-asan --enable-ubsan

  # plugin build rules do not work for sanitizer builds
  for DIR in 3rdparty lib src tools tests
  do
    make -C$DIR -j$NPROC
  done
  sudo make -Cdata install

  cd tests
  make check -j$NPROC
  ./post-install-test.sh

elif [[ -z "$1" ]]; then
  ./autogen.sh --with-download-instruments
  make -j$NPROC
  make check -j$NPROC
  sudo make install
else
  echo "usage: macbuild.sh [ sanitize ]"
fi

cd tests
./post-install-test.sh
cd ..
