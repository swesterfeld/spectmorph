#!/bin/bash
set -e
source ./config.sh
cd ..
docker build -t spectmorph -f static/Dockerfile .
cd static
OUT_DIR=spectmorph-$PACKAGE_VERSION-x86_64
rm -rf $OUT_DIR

# TEMPLATES / INSTRUMENTS
mkdir -p $OUT_DIR
docker run -v $PWD/$OUT_DIR:/data -t spectmorph cp -av \
  /usr/local/spectmorph/share/spectmorph/templates \
  /usr/local/spectmorph/share/spectmorph/instruments \
  /usr/local/spectmorph/share/spectmorph/fonts \
  /data

# VST
mkdir -p $OUT_DIR/vst
docker run -v $PWD/$OUT_DIR:/data -t spectmorph cp -av \
  vst/spectmorph_vst.so.static /data/vst/spectmorph_vst.so

# LV2
mkdir -p $OUT_DIR/lv2/spectmorph.lv2
docker run -v $PWD/$OUT_DIR:/data -t spectmorph cp -av \
  lv2/spectmorph_lv2.so.static /data/lv2/spectmorph.lv2/spectmorph_lv2.so
docker run -v $PWD/$OUT_DIR:/data -t spectmorph cp -av \
  /usr/local/spectmorph/lib/lv2/spectmorph.lv2/spectmorph.ttl \
  /usr/local/spectmorph/lib/lv2/spectmorph.lv2/manifest.ttl /data/lv2/spectmorph.lv2

strip $OUT_DIR/vst/spectmorph_vst.so $OUT_DIR/lv2/spectmorph.lv2/spectmorph_lv2.so

# INSTALLER
cp -aiv install.sh $OUT_DIR
chmod +x $OUT_DIR/install.sh

# ID
docker run -v $PWD/$OUT_DIR:/data -t spectmorph chown -R $(id -u).$(id -g) /data

# TARBALL
tar cfvJ $OUT_DIR.tar.xz $OUT_DIR
