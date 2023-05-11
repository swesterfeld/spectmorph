#!/bin/bash

source ./config.sh

die()
{
  echo "$0: $@"
  exit 1
}

set -x
make -j$(nproc) -C.. || die "make failed"
make -C.. install || die "make install failed"
cp -v $PREFIX/lib/vst/SpectMorph.dll . || die "error: cp SpectMorph.dll"
cp -v $PREFIX/lib/clap/SpectMorph.clap . || die "error: cp SpectMorph.clap"
cp -rv $PREFIX/lib/lv2/spectmorph.lv2 . || die "error: cp spectmorph.lv2"

for DIR in templates fonts
do
  mkdir -p ./$DIR
  rm ./$DIR/*
  cp -v $PREFIX/share/spectmorph/$DIR/* $DIR || die "error: cp $DIR"
done

# CHECK INSTRUMENT VERSION
echo -n "### Instruments: "
grep "version.*$PACKAGE_VERSION" instruments/standard/index.smindex || die "did not use appropriate instrument version: $PACKAGE_VERSION"
