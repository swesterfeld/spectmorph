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

make_pkg()
{
  SHORT=$1
  NAME=SpectMorph.$SHORT
  INSTDIR=$2
  SONAME=$3
  IDENT=$4

  rm -rf installer-tmp/$NAME
  mkdir -p installer-tmp/$NAME/$NAME/Contents/MacOS installer-tmp/$NAME/$NAME/Contents/Resources || die "error: mkdir SpectMorph.vst/..."
  lipo -create -output installer-tmp/$NAME/$NAME/Contents/MacOS/SpectMorph \
                       $SMDIR/lib/$SHORT/$SONAME \
                       $SMDIR_CROSS/lib/$SHORT/$SONAME || die "error: lipo SpectMorph"
  cp -rv PkgInfo installer-tmp/$NAME/$NAME/Contents || die "error: cp ...pkginfo"
  cat Info.plist | sed "s/vst/$SHORT/g" > installer-tmp/$NAME/$NAME/Contents/Info.plist || die "error: gen info"

  codesign -s "Developer ID Application: Stefan Westerfeld (ZA556HAPK8)" installer-tmp/$NAME/$NAME --timestamp

  pkgbuild --sign "Developer ID Installer: Stefan Westerfeld (ZA556HAPK8)" --root installer-tmp/$NAME --identifier $IDENT --version ${PACKAGE_VERSION} --install-location "$INSTDIR" "$NAME.pkg"

  rm -rf installer-tmp/$NAME
}

make_pkg vst /Library/Audio/Plug-Ins/VST SpectMorph.so org.spectmorph.vst.SpectMorph.pkg
make_pkg clap /Library/Audio/Plug-Ins/CLAP SpectMorph.clap org.spectmorph.clap.SpectMorph.pkg

mkdir -p installer-tmp/SpectMorph.data/SpectMorph
cp -rv instruments installer-tmp/SpectMorph.data/SpectMorph || die "error: cp instruments"
cp -rv $PREFIX/share/spectmorph/templates installer-tmp/SpectMorph.data/SpectMorph || die "error: cp templates"
pkgbuild --sign "Developer ID Installer: Stefan Westerfeld (ZA556HAPK8)" --root installer-tmp/SpectMorph.data --identifier "org.spectmorph.data.SpectMorph.pkg" --version ${PACKAGE_VERSION} --install-location "/tmp/SpectMorph.data" --scripts DataInstallerScript SpectMorph.data.pkg
rm -rf installer-tmp/SpectMorph.data

productbuild --synthesize --package SpectMorph.clap.pkg --package SpectMorph.vst.pkg --package SpectMorph.data.pkg SpectMorph.xml
productbuild --sign "Developer ID Installer: Stefan Westerfeld (ZA556HAPK8)" --distribution SpectMorph.xml SpectMorph-${PACKAGE_VERSION}.pkg

rm SpectMorph.xml SpectMorph.vst.pkg SpectMorph.clap.pkg SpectMorph.data.pkg

### xcrun notarytool submit SpectMorph-0.5.2-test1.pkg --apple-id stefan@space.twc.de --team-id ZA556HAPK8 --wait
### xcrun stapler staple SpectMorph-0.5.2-test1.pkg
