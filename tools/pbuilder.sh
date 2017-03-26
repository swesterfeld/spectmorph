#!/bin/bash

set -e

if [ “$(id -u)” != “0” ]; then
  echo “This script must be run as root” 2>&1
  exit 1
fi

#----------------------------------------
# CONFIG FILE CONTENTS: ~/.pbuilderrc
#----------------------------------------
# COMPONENTS="main restricted universe multiverse"
# CCACHEDIR=/var/cache/pbuilder/ccache
#----------------------------------------

DSC=../../spectmorph-minimal_0.3.2.dsc

unset CC
unset CXX

for DIST in xenial
do
  for ARCH in i386 amd64
  do
    BASETGZ=/var/cache/pbuilder/$DIST-$ARCH-base.tgz

    if test -f $BASETGZ; then
      echo using existing basetgz $BASETGZ
    else
      pbuilder --create                \
               --distribution $DIST    \
               --architecture $ARCH    \
               --basetgz $BASETGZ
    fi

    echo "----------------------------------------------------------------"
    echo "---- building from $DSC"
    echo "---- dist $DIST, arch $ARCH"
    echo "----------------------------------------------------------------"

    pbuilder --build                \
             --distribution $DIST   \
             --architecture $ARCH   \
             --basetgz $BASETGZ     \
             $DSC
  done
done
