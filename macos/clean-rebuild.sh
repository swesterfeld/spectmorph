#!/bin/bash

set -e

git pull
pushd ../../cross/spectmorph
git pull
popd

pushd ../../cross/spectmorph/macos
./build-deps.sh x86_64 clean
./build.sh x86_64 clean
popd

rm -rf prefix build
./build-deps.sh aarch64 clean
./build.sh aarch64 clean
./dist.sh
