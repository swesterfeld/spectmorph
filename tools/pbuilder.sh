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

DSC=../../spectmorph_0.5.2.dsc

unset CC
unset CXX
export DEB_BUILD_OPTIONS="parallel=$(nproc)"

for NAME in \
  xenial-i386 xenial-amd64 \
  bionic-i386 bionic-amd64 \
  focal-amd64
do
  DIST=${NAME%%-*}
  ARCH=${NAME#*-}

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
