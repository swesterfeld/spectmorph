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

make_pkg()
{
  NAME=$1
  INSTDIR=$2
  IDENT=$3

  codesign -s "Developer ID Application: Stefan Westerfeld (ZA556HAPK8)" $NAME --timestamp

  rm -rf installer-tmp/$NAME
  mkdir -p installer-tmp/$NAME

  mv $NAME installer-tmp/$NAME
  pkgbuild --sign "Developer ID Installer: Stefan Westerfeld (ZA556HAPK8)" --root installer-tmp/$NAME --identifier $IDENT --version ${PACKAGE_VERSION} --install-location "$INSTDIR" "$NAME.pkg"

  rm -rf installer-tmp/$NAME
}

make_pkg SpectMorph.vst /Library/Audio/Plug-Ins/VST org.spectmorph.vst.SpectMorph.pkg
make_pkg SpectMorph.clap /Library/Audio/Plug-Ins/CLAP org.spectmorph.clap.SpectMorph.pkg

productbuild --synthesize --package SpectMorph.clap.pkg --package SpectMorph.vst.pkg SpectMorph.xml
productbuild --sign "Developer ID Installer: Stefan Westerfeld (ZA556HAPK8)" --distribution SpectMorph.xml SpectMorph-${PACKAGE_VERSION}.pkg

rm -f SpectMorph-${PACKAGE_VERSION}.zip
zip -r SpectMorph-${PACKAGE_VERSION}.zip SpectMorph-${PACKAGE_VERSION}.pkg || die "error: zip"

rm SpectMorph.xml SpectMorph.vst.pkg SpectMorph.clap.pkg
