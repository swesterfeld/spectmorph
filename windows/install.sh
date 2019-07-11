#!/bin/bash

source ./config.sh

die()
{
  echo "$0: $@"
  exit 1
}

./build.sh || die "build failed"
./mk-setup.sh || die "mk-setup failed"
start setup-spectmorph-${PACKAGE_VERSION}.exe || die "setup.exe failed"
