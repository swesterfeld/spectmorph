#!/bin/bash

set -e

sudo apt-get install -y gettext libsndfile1-dev libfftw3-dev libgl-dev libglib2.0-dev
./autogen.sh --without-ao
cd static
source ./config.sh

OUT_DIR=spectmorph-$PACKAGE_VERSION-x86_64
rm -rf $OUT_DIR
mkdir -p $OUT_DIR

cd ..
SRC_DIR=$PWD
docker build -t spectmorph-static -f static/Dockerfile .
cd static

docker run -v $SRC_DIR:/output -v $PWD/$OUT_DIR:/data --rm -t spectmorph-static static/staticbuild.sh
docker run -v $PWD/$OUT_DIR:/data -t spectmorph-static misc/statictest.sh
