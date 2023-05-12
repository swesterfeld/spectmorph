#!/bin/bash
set -e
source ./config.sh

OUT_DIR=spectmorph-$PACKAGE_VERSION-x86_64
rm -rf $OUT_DIR
mkdir -p $OUT_DIR

cd ..
docker build -t spectmorph -f static/Dockerfile .
cd static

# TEMPLATES / INSTRUMENTS
docker run -v $PWD/$OUT_DIR:/data -t spectmorph cp -av \
  /usr/local/spectmorph/share/spectmorph/templates \
  /usr/local/spectmorph/share/spectmorph/instruments \
  /usr/local/spectmorph/share/spectmorph/fonts \
  /data

# VST
mkdir -p $OUT_DIR/vst
docker run -v $PWD/$OUT_DIR:/data -t spectmorph cp -av \
  vst/spectmorph_vst.so.static /data/vst/spectmorph_vst.so

# CLAP
mkdir -p $OUT_DIR/clap
docker run -v $PWD/$OUT_DIR:/data -t spectmorph cp -av \
  clap/SpectMorph.clap.static /data/clap/SpectMorph.clap

# LV2
mkdir -p $OUT_DIR/lv2/spectmorph.lv2
docker run -v $PWD/$OUT_DIR:/data -t spectmorph cp -av \
  lv2/spectmorph_lv2.so.static /data/lv2/spectmorph.lv2/spectmorph_lv2.so
docker run -v $PWD/$OUT_DIR:/data -t spectmorph cp -av \
  /usr/local/spectmorph/lib/lv2/spectmorph.lv2/spectmorph.ttl \
  /usr/local/spectmorph/lib/lv2/spectmorph.lv2/manifest.ttl /data/lv2/spectmorph.lv2

# ID
docker run -v $PWD/$OUT_DIR:/data -t spectmorph chown -R $(id -u).$(id -g) /data

strip $OUT_DIR/vst/spectmorph_vst.so $OUT_DIR/lv2/spectmorph.lv2/spectmorph_lv2.so $OUT_DIR/clap/SpectMorph.clap

# INSTALLER
cp -aiv install.sh $OUT_DIR
chmod +x $OUT_DIR/install.sh

# CHECK INSTRUMENT VERSION
echo -n "### Instruments: "
grep "version \"$PACKAGE_VERSION\"" $OUT_DIR/instruments/standard/index.smindex || {
  echo "### Did not use appropriate instrument version: $PACKAGE_VERSION ###"
  exit 1
}

# TARBALL
tar cfvJ $OUT_DIR.tar.xz $OUT_DIR
