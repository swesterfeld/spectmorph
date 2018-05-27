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
	echo "--- Downloading.. $2"
	test -f ${SRCDIR}/$1 || curl -L -o ${SRCDIR}/$1 $2
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

PREFIX=$PWD/prefix
GLOBAL_CFLAGS="-isysroot /Applications/Xcode.app/Contents//Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk"
GLOBAL_LDFLAGS="-L$PREFIX/lib"
MAKEFLAGS="-j9"
PATH=$PWD/prefix/bin:$PATH

src zlib-1.2.7 tar.gz ftp://ftp.simplesystems.org/pub/libpng/png/src/history/zlib/zlib-1.2.7.tar.gz

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
autoconfbuild --disable-fortran --enable-single

src libffi-3.2.1 tar.gz https://sourceware.org/pub/libffi/libffi-3.2.1.tar.gz
autoconfbuild


src gettext-0.19.8.1 tar.xz https://ftp.gnu.org/gnu/gettext/gettext-0.19.8.1.tar.xz
autoconfbuild "--disable-silent-rules" "--disable-debug" \
              "--with-included-gettext" "--with-included-glib" "--with-included-libcroco" "--with-included-libunistring" "--with-emacs" \
              "--disable-java" "--disable-csharp" "--without-git" "--without-cvs" "--without-xz"

src glib-2.56.1 tar.xz https://download.gnome.org/sources/glib/2.56/glib-2.56.1.tar.xz
autoconfbuild --enable-static

src freetype-2.5.3 tar.gz http://download.savannah.gnu.org/releases/freetype/freetype-2.5.3.tar.gz
autoconfbuild --with-harfbuzz=no --with-png=no --with-bzip2=no

src pixman-0.34.0 tar.gz https://www.cairographics.org/releases/pixman-0.34.0.tar.gz
autoconfbuild

src cairo-1.15.10 tar.xz https://cairographics.org/snapshots/cairo-1.15.10.tar.xz
autoconfbuild


