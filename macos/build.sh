#!/bin/bash


die()
{
  echo "$0: $@"
  exit 1
}

set -x

. sdk-options.sh

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
./autogen.sh --prefix=$SM_PREFIX --without-qt --without-lv2 --without-jack || die "configure failed"
cd $MACOS_DIR || die "cd macos_dir"

source ./config.sh || die "source config.sh"

make -j9 -C.. || die "make failed"
make -C.. install || die "make install failed"

### VST
rm -rf ./SpectMorph.vst/*
mkdir -p SpectMorph.vst/Contents/MacOS SpectMorph.vst/Contents/Resources || die "error: mkdir SpectMorph.vst/..."
cp -v $PREFIX/lib/vst/SpectMorph.so SpectMorph.vst/Contents/MacOS/SpectMorph || die "error: cp SpectMorph.so"
cp -rv $PREFIX/share/spectmorph/templates SpectMorph.vst/Contents/Resources || die "error: cp templates"
cp -rv instruments SpectMorph.vst/Contents/Resources || die "error: cp instruments"
cp -rv Info.plist PkgInfo SpectMorph.vst/Contents || die "error: cp ...info"

### CLAP
rm -rf ./SpectMorph.clap/*
mkdir -p SpectMorph.clap/Contents/MacOS SpectMorph.clap/Contents/Resources || die "error: mkdir SpectMorph.clap/..."
cp -v $PREFIX/lib/clap/SpectMorph.clap SpectMorph.clap/Contents/MacOS/SpectMorph || die "error: cp SpectMorph.clap"
cp -rv $PREFIX/share/spectmorph/templates SpectMorph.clap/Contents/Resources || die "error: cp templates"
cp -rv instruments SpectMorph.clap/Contents/Resources || die "error: cp instruments"
cp -rv PkgInfo SpectMorph.clap/Contents || die "error: cp ...pkginfo"
cat Info.plist | sed s/vst/clap/g > SpectMorph.clap/Contents/Info.plist || die "error: gen info"

rm -f SpectMorph-${PACKAGE_VERSION}.zip
zip -r SpectMorph-${PACKAGE_VERSION}.zip SpectMorph.vst SpectMorph.clap || die "error: zip"

# unzip SpectMorph-0.4.0.zip -d ~/Library/Audio/Plug-Ins/VST
