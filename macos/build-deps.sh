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
	autoconfconf $@
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

mkdir -p $SRCDIR $BUILDD

. sdk-options.sh
PREFIX=$PWD/prefix
GLOBAL_CFLAGS="-isysroot $SDK_DIRECTORY -mmacosx-version-min=$SDK_MINVERSION"
GLOBAL_LDFLAGS="-L$PREFIX/lib"
MAKEFLAGS="-j9"
PATH=$PWD/prefix/bin:$PATH
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH

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
./configure --prefix=$PREFIX --disable-shared --with-zlib-prefix=$PREFIX
make $MAKEFLAGS
make install

src fftw-3.3.7 tar.gz http://www.fftw.org/fftw-3.3.7.tar.gz
autoconfbuild --disable-fortran --enable-single --enable-threads --with-combined-threads

src libffi-3.4.2 tar.gz https://github.com/libffi/libffi/archive/refs/tags/v3.4.2.tar.gz
./autogen.sh
autoconfbuild

src gettext-0.19.8.1 tar.xz https://ftp.gnu.org/gnu/gettext/gettext-0.19.8.1.tar.xz
autoconfbuild "--disable-silent-rules" "--disable-debug" \
              "--with-included-gettext" "--with-included-glib" "--with-included-libcroco" "--with-included-libunistring" "--with-emacs" \
              "--disable-java" "--disable-csharp" "--without-git" "--without-cvs" "--without-xz"

src glib-2.56.1 tar.xz https://download.gnome.org/sources/glib/2.56/glib-2.56.1.tar.xz
autoconfbuild --enable-static -enable-utf --enable-unicode-properties --with-pcre=internal

src freetype-2.5.5 tar.gz https://sourceforge.net/projects/freetype/files/freetype2/2.5.5/freetype-2.5.5.tar.gz
autoconfbuild --with-harfbuzz=no --with-png=no --with-bzip2=no

src pixman-0.34.0 tar.gz https://www.cairographics.org/releases/pixman-0.34.0.tar.gz
patch -p1 < ../../pixman-0.34.0.diff
autoconfbuild

src cairo-1.15.10 tar.xz https://cairographics.org/snapshots/cairo-1.15.10.tar.xz
autoconfbuild

src libvorbis-1.3.6 tar.xz http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.6.tar.xz
autoconfbuild

src libogg-1.3.3 tar.xz http://downloads.xiph.org/releases/ogg/libogg-1.3.3.tar.xz
autoconfbuild

src flac-1.3.2 tar.xz https://ftp.osuosl.org/pub/xiph/releases/flac/flac-1.3.2.tar.xz
autoconfbuild --enable-static

src libsndfile-1.0.28 tar.gz http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28.tar.gz
autoconfbuild
