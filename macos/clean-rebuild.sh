#!/bin/bash

set -e

git pull
pushd ../../cross/spectmorph
git pull
popd

pushd ../../cross/spectmorph/macos
if [ "x$1" != "xnodeps" ]; then
  rm -rf prefix build
  ./build-deps.sh x86_64 clean
fi
./build.sh x86_64 clean
popd

if [ "x$1" != "xnodeps" ]; then
  rm -rf prefix build
  ./build-deps.sh aarch64 clean
fi
./build.sh aarch64 clean
./dist.sh
