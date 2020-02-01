#!/bin/bash

set -e

if [ “$(id -u)” != “0” ]; then
  echo "This script must be run as root." 2>&1
  exec su -c "$0"
fi

#----------------------------------------
# CONFIG FILE CONTENTS: ~/.pbuilderrc
#----------------------------------------
# COMPONENTS="main restricted universe multiverse"
# CCACHEDIR=/var/cache/pbuilder/ccache
#----------------------------------------

DSC=../../spectmorph_0.5.1.dsc

unset CC
unset CXX
export DEB_BUILD_OPTIONS="parallel=$(nproc)"

for DIST in xenial bionic
do
  for ARCH in i386 amd64
  do
    NAME="$DIST-$ARCH"
    BASETGZ="/var/cache/pbuilder/$NAME/base.tgz"
    BUILDRESULT="/var/cache/pbuilder/$NAME/result/"
    APTCACHE="/var/cache/pbuilder/$NAME/aptcache/"
    mkdir -p "$BUILDRESULT" "$APTCACHE"

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
             --buildresult $BUILDRESULT  \
             --aptcache $APTCACHE   \
             $DSC
  done
done
