#!/bin/bash


die()
{
  echo "$0: $@"
  exit 1
}

set -x

export PREFIX=$PWD/prefix
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH
export MACOS_DIR=$PWD
cd ..
CFLAGS="-isysroot /Applications/Xcode.app/Contents//Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk" \
CXXFLAGS="-isysroot /Applications/Xcode.app/Contents//Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk" \
OBJCFLAGS="-isysroot /Applications/Xcode.app/Contents//Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk" \
OBJCXXFLAGS="-isysroot /Applications/Xcode.app/Contents//Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk" \
./autogen.sh --prefix=/usr/local/spectmorph --without-qt --without-lv2 --without-jack || die "configure failed"
cd $MACOS_DIR || die "cd macos_dir"

source ./config.sh || die "source config.sh"

make -j9 -C.. || die "make failed"
make -C.. install || die "make install failed"
rm -rf ./SpectMorph.vst/*
mkdir -p SpectMorph.vst/Contents/MacOS SpectMorph.vst/Contents/Resources || die "error: mkdir SpectMorph.vst/..."
cp -v $PREFIX/lib/vst/SpectMorph.so SpectMorph.vst/Contents/MacOS/SpectMorph || die "error: cp SpectMorph.so"
cp -rv $PREFIX/share/spectmorph/templates SpectMorph.vst/Contents/Resources || die "error: cp templates"
cp -rv instruments SpectMorph.vst/Contents/Resources || die "error: cp instruments"
cp -rv Info.plist PkgInfo SpectMorph.vst/Contents || die "error: cp ...info"
rm -f SpectMorph-${PACKAGE_VERSION}.zip
zip -r SpectMorph-${PACKAGE_VERSION}.zip SpectMorph.vst || die "error: zip"

# unzip SpectMorph-0.4.0.zip -d ~/Library/Audio/Plug-Ins/VST
