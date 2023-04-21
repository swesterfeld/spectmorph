#!/bin/bash

# build all dependency libs, based on a script written by Robin Gareus
# https://github.com/zynaddsubfx/zyn-build-osx/blob/master/01_compile.sh

function autoconfconf {
set -e
echo "======= $(pwd) ======="
	CPPFLAGS="-I${PREFIX}/include${GLOBAL_CPPFLAGS:+ $GLOBAL_CPPFLAGS}" \
	CFLAGS="${GLOBAL_CFLAGS:+ $GLOBAL_CFLAGS}" \
	CXXFLAGS="${GLOBAL_CXXFLAGS:+ $GLOBAL_CXXFLAGS}" \
	LDFLAGS="${GLOBAL_LDFLAGS:+ $GLOBAL_LDFLAGS}" \
	./configure --disable-dependency-tracking --prefix=$PREFIX $@
}

function autoconfbuild {
set -e
	autoconfconf $@ $SDK_AUTOCONF_BUILD
	make $MAKEFLAGS
	make install
}

function download {
	echo "--- Downloading.. ${SRCDIR}/$1 $2"
	test -f ${SRCDIR}/$1 || curl -k -L -o ${SRCDIR}/$1 $2
}

function src {
	download ${1}.${2} $3
	cd ${BUILDD}
	rm -rf $1
	tar xf ${SRCDIR}/${1}.${2}
	cd $1
}

SRCDIR=$PWD/src
BUILDD=$PWD/build

export SDK_TARGET=$1
. sdk-options.sh

if [ -z "$SDK_AUTOCONF_BUILD" ]; then
  echo "target arch '$SDK_TARGET' not supported"
  exit 1
fi

PREFIX=$PWD/prefix
GLOBAL_CFLAGS="$SDK_OPTIONS"
GLOBAL_CXXFLAGS="$SDK_OPTIONS"
GLOBAL_LDFLAGS="-L$PREFIX/lib"
MAKEFLAGS="-j9"
PATH=$PWD/prefix/bin:$PATH
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH
export PKG_CONFIG="pkg-config --static"

if [ "x$2" = "xclean" ]; then
  rm -rf $PREFIX $BUILDD
fi

mkdir -p $SRCDIR $BUILDD

src zlib-1.2.11 tar.gz https://github.com/madler/zlib/archive/refs/tags/v1.2.11.tar.gz

CFLAGS="${GLOBAL_CFLAGS}" \
LDFLAGS="${GLOBAL_LDFLAGS}" \
./configure --prefix=$PREFIX --static
make $MAKEFLAGS
make install

src libpng-1.6.34 tar.xz https://downloads.sourceforge.net/project/libpng/libpng16/1.6.34/libpng-1.6.34.tar.xz

CFLAGS="${GLOBAL_CFLAGS}" \
CPPFLAGS="-I$PREFIX/include" \
LDFLAGS="${GLOBAL_LDFLAGS}" \
./configure $SDK_AUTOCONF_BUILD --prefix=$PREFIX --disable-shared --with-zlib-prefix=$PREFIX
make $MAKEFLAGS
make install

src fftw-3.3.7 tar.gz http://www.fftw.org/fftw-3.3.7.tar.gz
autoconfbuild --disable-fortran --enable-single --enable-threads --with-combined-threads

src libffi-3.4.2 tar.gz https://github.com/libffi/libffi/releases/download/v3.4.2/libffi-3.4.2.tar.gz
autoconfbuild --disable-shared

src gettext-0.19.8.1 tar.xz https://ftp.gnu.org/gnu/gettext/gettext-0.19.8.1.tar.xz
autoconfbuild --disable-shared "--disable-silent-rules" "--disable-debug" \
              "--with-included-gettext" "--with-included-glib" "--with-included-libcroco" "--with-included-libunistring" "--with-emacs" \
              "--disable-java" "--disable-csharp" "--without-git" "--without-cvs" "--without-xz"

src glib-2.56.1 tar.xz https://download.gnome.org/sources/glib/2.56/glib-2.56.1.tar.xz
autoconfbuild --disable-shared --with-pcre=internal --disable-compile-warnings

src freetype-2.5.5 tar.gz https://sourceforge.net/projects/freetype/files/freetype2/2.5.5/freetype-2.5.5.tar.gz
autoconfbuild --disable-shared --with-harfbuzz=no --with-png=no --with-bzip2=no

src pixman-0.34.0 tar.gz https://www.cairographics.org/releases/pixman-0.34.0.tar.gz
patch -p1 < ../../pixman-0.34.0-notest.diff
autoreconf -i
autoconfbuild --disable-shared

src cairo-1.15.10 tar.xz https://cairographics.org/snapshots/cairo-1.15.10.tar.xz
patch -p1 < ../../cairo-1.15.10-notest.diff
autoreconf -i
autoconfbuild --disable-shared --disable-xlib --disable-xcb

src libogg-1.3.5 tar.xz https://github.com/xiph/ogg/releases/download/v1.3.5/libogg-1.3.5.tar.xz
autoconfbuild --disable-shared

src libvorbis-1.3.7 tar.xz https://github.com/xiph/vorbis/releases/download/v1.3.7/libvorbis-1.3.7.tar.xz
autoconfbuild --disable-shared

src flac-1.4.2 tar.xz https://github.com/xiph/flac/releases/download/1.4.2/flac-1.4.2.tar.xz
autoconfbuild --disable-shared

src opus-1.3.1 tar.gz https://archive.mozilla.org/pub/opus/opus-1.3.1.tar.gz
autoconfbuild --disable-shared

src libsndfile-1.2.0 tar.xz https://github.com/libsndfile/libsndfile/releases/download/1.2.0/libsndfile-1.2.0.tar.xz
autoconfbuild --disable-shared

src lv2-1.18.10 tar.gz https://github.com/lv2/lv2/archive/refs/tags/v1.18.10.tar.gz
meson setup build --prefix $PREFIX
cd build
meson compile
meson install
