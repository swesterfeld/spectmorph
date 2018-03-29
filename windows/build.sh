#!/bin/bash

die()
{
  echo "$0: $@"
  exit 1
}

set -x
make -j9 -C.. || die "make failed"
make -C.. install || die "make install failed"
cp -v /mingw64/lib/vst/SpectMorph.dll .
rm ./templates/*
cp -v /mingw64/share/spectmorph/templates/* templates
