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

for DIR in templates fonts
do
  mkdir -p ./$DIR
  rm ./$DIR/*
  cp -v $PREFIX/share/spectmorph/$DIR/* $DIR || die "error: cp $DIR"
done
