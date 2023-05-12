#!/bin/bash

set -e

if [ “$(id -u)” == “0” ] && [ "x$1" != "xroot" ]; then
  echo "This script must be started as user."
  exit 1
fi

if [ “$(id -u)” != “0” ]; then
  pushd ..
  git pull
  dpkg-buildpackage --no-sign -S
  popd

  exec su -c "$0 root"
fi

#----------------------------------------
# CONFIG FILE CONTENTS: ~/.pbuilderrc
#----------------------------------------
# COMPONENTS="main restricted universe multiverse"
# CCACHEDIR=/var/cache/pbuilder/ccache
#----------------------------------------

VERSION=0.6.0
DSC=../../spectmorph_${VERSION}.dsc
if [ ! -f "../data/spectmorph-instruments-${VERSION}.tar.xz" ]; then
  echo "You need to have an instrument tarball installed to build a package"
  exit 1
fi

echo -n "### Instruments: "
tar xf ../data/spectmorph-instruments-${VERSION}.tar.xz instruments/standard/index.smindex -O | grep "version \"${VERSION}\"" || {
  echo instruments not valid
  exit 1
}

unset CC
unset CXX
export DEB_BUILD_OPTIONS="parallel=$(nproc)"
export DISTS="focal-amd64 jammy-amd64"
for NAME in $DISTS
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

for NAME in $DISTS
do
  ls -l /var/cache/pbuilder/$NAME/result/spectmorph_${VERSION}*deb
done
