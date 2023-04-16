#!/bin/bash


die()
{
  echo "$0: $@"
  exit 1
}

set -x

source config.sh

SMDIR=spectmorph
SMDIR_CROSS=../../cross/spectmorph/macos/spectmorph

### VST
rm -rf ./SpectMorph.vst/*
mkdir -p SpectMorph.vst/Contents/MacOS SpectMorph.vst/Contents/Resources || die "error: mkdir SpectMorph.vst/..."
lipo -create -output SpectMorph.vst/Contents/MacOS/SpectMorph \
                     $SMDIR/lib/vst/SpectMorph.so \
                     $SMDIR_CROSS/lib/vst/SpectMorph.so || die "error: lipo SpectMorph"
cp -rv $PREFIX/share/spectmorph/templates SpectMorph.vst/Contents/Resources || die "error: cp templates"
cp -rv instruments SpectMorph.vst/Contents/Resources || die "error: cp instruments"
cp -rv Info.plist PkgInfo SpectMorph.vst/Contents || die "error: cp ...info"

### CLAP
rm -rf ./SpectMorph.clap/*
mkdir -p SpectMorph.clap/Contents/MacOS SpectMorph.clap/Contents/Resources || die "error: mkdir SpectMorph.clap/..."
lipo -create -output SpectMorph.clap/Contents/MacOS/SpectMorph \
                     $SMDIR/lib/clap/SpectMorph.clap \
                     $SMDIR_CROSS/lib/clap/SpectMorph.clap || die "error: lipo SpectMorph"
cp -rv $PREFIX/share/spectmorph/templates SpectMorph.clap/Contents/Resources || die "error: cp templates"
cp -rv instruments SpectMorph.clap/Contents/Resources || die "error: cp instruments"
cp -rv PkgInfo SpectMorph.clap/Contents || die "error: cp ...pkginfo"
cat Info.plist | sed s/vst/clap/g > SpectMorph.clap/Contents/Info.plist || die "error: gen info"

## codesign
codesign -s "Developer ID Application: Stefan Westerfeld (ZA556HAPK8)" SpectMorph.vst --timestamp
codesign -s "Developer ID Application: Stefan Westerfeld (ZA556HAPK8)" SpectMorph.clap --timestamp

rm -f SpectMorph-${PACKAGE_VERSION}.zip
zip -r SpectMorph-${PACKAGE_VERSION}.zip SpectMorph.vst SpectMorph.clap || die "error: zip"

# cp -av SpectMorph.vst ~/Library/Audio/Plug-Ins/VST
# cp -av SpectMorph.clap ~/Library/Audio/Plug-Ins/CLAP
