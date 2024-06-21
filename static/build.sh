#!/bin/bash
set -e
source ./config.sh

OUT_DIR=spectmorph-$PACKAGE_VERSION-x86_64
rm -rf $OUT_DIR
mkdir -p $OUT_DIR

cd ..
SRC_DIR=$PWD
docker build -t spectmorph-static -f static/Dockerfile .
cd static

docker run -v $SRC_DIR:/output -v $PWD/$OUT_DIR:/data --rm -t spectmorph-static static/staticbuild.sh

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
rm -r $OUT_DIR
