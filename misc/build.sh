#!/bin/bash
set -Eeuo pipefail

build()
{
  if [ -f "./Makefile" ]; then
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

export CCACHE_DIR=/output/ccache
export CCACHE_MAXSIZE=1G

# Tests using gcc
export CC="ccache gcc" CXX="ccache g++"

build "$@" --enable-debug-cxx

if [ $# -eq 0 ]; then
  lv2lint http://spectmorph.org/plugins/spectmorph
fi

make -j `nproc` distcheck

# Tests clang
export CC="ccache clang"  CXX="ccache clang++"

build "$@"

if [ $# -eq 0 ]; then
  lv2lint http://spectmorph.org/plugins/spectmorph
fi
