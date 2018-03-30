#!/bin/bash

source ./config.sh

die()
{
  echo "$0: $@"
  exit 1
}

set -x
make -j9 -C.. || die "make failed"
make -C.. install || die "make install failed"
cp -v $PREFIX/lib/vst/SpectMorph.dll . || die "error: cp SpectMorph.dll"
mkdir -p ./templates
rm ./templates/*
cp -v $PREFIX/share/spectmorph/templates/* templates || die "error: cp templates"
