#!/bin/bash


die()
{
  echo "$0: $@"
  exit 1
}

set -x
set -e

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

make_pkg vst /Library/Audio/Plug-Ins/VST SpectMorph.dylib org.spectmorph.vst.SpectMorph.pkg
make_pkg clap /Library/Audio/Plug-Ins/CLAP SpectMorph.clap org.spectmorph.clap.SpectMorph.pkg

rm -rf installer-tmp/SpectMorph.lv2
mkdir -p installer-tmp/SpectMorph.lv2/spectmorph.lv2
cp $SMDIR/lib/lv2/spectmorph.lv2/*.ttl installer-tmp/SpectMorph.lv2/spectmorph.lv2
lipo -create -output installer-tmp/SpectMorph.lv2/spectmorph.lv2/spectmorph_lv2.dylib \
  $SMDIR/lib/lv2/spectmorph.lv2/spectmorph_lv2.dylib \
  $SMDIR_CROSS/lib/lv2/spectmorph.lv2/spectmorph_lv2.dylib
codesign -s "Developer ID Application: Stefan Westerfeld (ZA556HAPK8)" installer-tmp/SpectMorph.lv2/spectmorph.lv2/* --timestamp
pkgbuild --sign "Developer ID Installer: Stefan Westerfeld (ZA556HAPK8)" --root installer-tmp/SpectMorph.lv2 \
         --identifier org.spectmorph.lv2.SpectMorph.pkg --version ${PACKAGE_VERSION} --install-location /Library/Audio/Plug-Ins/LV2 "SpectMorph.lv2.pkg"

mkdir -p installer-tmp/SpectMorph.data/SpectMorph
cp -rv instruments installer-tmp/SpectMorph.data/SpectMorph || die "error: cp instruments"
cp -rv $PREFIX/share/spectmorph/templates installer-tmp/SpectMorph.data/SpectMorph || die "error: cp templates"
pkgbuild --sign "Developer ID Installer: Stefan Westerfeld (ZA556HAPK8)" --root installer-tmp/SpectMorph.data --identifier "org.spectmorph.data.SpectMorph.pkg" --version ${PACKAGE_VERSION} --install-location "/tmp/SpectMorph.data" --scripts DataInstallerScript SpectMorph.data.pkg
rm -rf installer-tmp/SpectMorph.data

cat > installer-tmp/distribution.xml << EOH
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
    <title>SpectMorph ${PACKAGE_VERSION}</title>
    <license file="$PWD/gpl-3.0.rtf" />
    <pkg-ref id="org.spectmorph.clap.SpectMorph.pkg"/>
    <pkg-ref id="org.spectmorph.lv2.SpectMorph.pkg"/>
    <pkg-ref id="org.spectmorph.vst.SpectMorph.pkg"/>
    <pkg-ref id="org.spectmorph.data.SpectMorph.pkg"/>
    <options customize="never" require-scripts="false" hostArchitectures="x86_64,arm64" rootVolumeOnly="true"/>
    <domains enable_anywhere="false" enable_currentUserHome="false" enable_localSystem="true"/>
    <choices-outline>
        <line choice="default">
            <line choice="org.spectmorph.clap.SpectMorph.pkg"/>
            <line choice="org.spectmorph.lv2.SpectMorph.pkg"/>
            <line choice="org.spectmorph.vst.SpectMorph.pkg"/>
            <line choice="org.spectmorph.data.SpectMorph.pkg"/>
        </line>
    </choices-outline>
    <choice id="default"/>

    <choice id="org.spectmorph.clap.SpectMorph.pkg" visible="false">
        <pkg-ref id="org.spectmorph.clap.SpectMorph.pkg"/>
    </choice>
    <pkg-ref id="org.spectmorph.clap.SpectMorph.pkg" version="${PACKAGE_VERSION}" onConclusion="none">SpectMorph.clap.pkg</pkg-ref>

    <choice id="org.spectmorph.lv2.SpectMorph.pkg" visible="false">
        <pkg-ref id="org.spectmorph.lv2.SpectMorph.pkg"/>
    </choice>
    <pkg-ref id="org.spectmorph.lv2.SpectMorph.pkg" version="${PACKAGE_VERSION}" onConclusion="none">SpectMorph.lv2.pkg</pkg-ref>

    <choice id="org.spectmorph.vst.SpectMorph.pkg" visible="false">
        <pkg-ref id="org.spectmorph.vst.SpectMorph.pkg"/>
    </choice>
    <pkg-ref id="org.spectmorph.vst.SpectMorph.pkg" version="${PACKAGE_VERSION}" onConclusion="none">SpectMorph.vst.pkg</pkg-ref>

    <choice id="org.spectmorph.data.SpectMorph.pkg" visible="false">
        <pkg-ref id="org.spectmorph.data.SpectMorph.pkg"/>
    </choice>
    <pkg-ref id="org.spectmorph.data.SpectMorph.pkg" version="${PACKAGE_VERSION}" onConclusion="none">SpectMorph.data.pkg</pkg-ref>
</installer-gui-script>
EOH
productbuild --sign "Developer ID Installer: Stefan Westerfeld (ZA556HAPK8)" --distribution installer-tmp/distribution.xml SpectMorph-${PACKAGE_VERSION}.pkg

rm -rf installer-tmp SpectMorph.clap.pkg SpectMorph.lv2.pkg SpectMorph.vst.pkg SpectMorph.data.pkg

xcrun notarytool submit SpectMorph-${PACKAGE_VERSION}.pkg --apple-id stefan@space.twc.de --team-id ZA556HAPK8 --password "$APPLE_NOTARIZATION_PASSWORD" --wait
xcrun stapler staple SpectMorph-${PACKAGE_VERSION}.pkg
