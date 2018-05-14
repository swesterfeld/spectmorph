#!/bin/bash

source ./config.sh

die()
{
  echo "$0: $@"
  exit 1
}

set -x
make -j9 -C.. || die "make failed"
make -C.. install || die "make install failed"
rm -rf ./SpectMorph.vst/*
mkdir -p SpectMorph.vst/Contents/MacOS SpectMorph.vst/Contents/Resources || die "error: mkdir SpectMorph.vst/..."
cp -v $PREFIX/lib/vst/SpectMorph.so SpectMorph.vst/Contents/MacOS/SpectMorph || die "error: cp SpectMorph.so"
cp -rv $PREFIX/share/spectmorph/templates SpectMorph.vst/Contents/Resources || die "error: cp templates"
cp -rv instruments SpectMorph.vst/Contents/Resources || die "error: cp instruments"
cp -rv Info.plist PkgInfo SpectMorph.vst/Contents || die "error: cp ...info"
zip -r SpectMorph-${PACKAGE_VERSION}.zip SpectMorph.vst || die "error: zip"

# unzip SpectMorph-0.4.0.zip -d ~/Library/Audio/Plug-Ins/VST
