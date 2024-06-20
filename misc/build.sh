#!/bin/bash
set -Eeuo pipefail

build()
{
  if [ -f "./configure" ]; then
    make uninstall
    make distclean
  fi
  echo "###############################################################################"
  echo "# BUILD TESTS :"
  echo "#   CC=$CC CXX=$CXX "
  echo "#   ./autogen.sh $@"
  echo "###############################################################################"
  $CXX --version | sed '/^[[:space:]]*$/d;s/^/#   /'
  echo "###############################################################################"
  ./autogen.sh "$@"
  make -j `nproc` V=1
  if ! make -j `nproc` check; then
    for LOG in $(find tests -iname '*.log')
    do
      echo "===== $LOG ====="
      cat $LOG
    done
    tar cJf /output/test-logs.tar.xz $(find tests -iname '*.log')
    exit 1
  fi
  make install
}

# fail in make check if UB sanitizer produces error
export UBSAN_OPTIONS=halt_on_error=1

# Tests using gcc
export CC=gcc CXX=g++

build "$@" --enable-debug-cxx

[ $# -eq 0 ] && lv2lint http://spectmorph.org/plugins/spectmorph

make -j `nproc` distcheck

# Tests clang
export CC=clang CXX=clang++

build "$@"

[ $# -eq 0 ] && lv2lint http://spectmorph.org/plugins/spectmorph
