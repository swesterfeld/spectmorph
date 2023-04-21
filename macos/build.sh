#!/bin/bash


die()
{
  echo "$0: $@"
  exit 1
}

set -x

export SDK_TARGET=$1
. sdk-options.sh

if [ -z "$SDK_AUTOCONF_BUILD" ]; then
  echo "target arch '$SDK_TARGET' not supported"
  exit 1
fi

export DEPS_PREFIX=$PWD/prefix
export SM_PREFIX=$PWD/spectmorph
export PKG_CONFIG_PATH=$DEPS_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH
export PKG_CONFIG="pkg-config --static"
export MACOS_DIR=$PWD
cd ..
CFLAGS="$SDK_OPTIONS -I$DEPS_PREFIX/include" \
CXXFLAGS="$CFLAGS" \
OBJCFLAGS="$CFLAGS" \
OBJCXXFLAGS="$CFLAGS" \
./autogen.sh $SDK_AUTOCONF_BUILD --prefix=$SM_PREFIX --without-qt --without-jack --without-ao --disable-shared || die "configure failed"
cd $MACOS_DIR || die "cd macos_dir"

source ./config.sh || die "source config.sh"

if [ "x$2" = "xclean" ]; then
  make -C.. clean || die "make clean failed"
fi

make -j9 -C.. || die "make failed"
make -C.. install || die "make install failed"
