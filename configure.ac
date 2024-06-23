AC_INIT([spectmorph],[1.0.0-beta1])
INST_TAG=2024-06-22
INST_SHA1SUM=36a9734ab91133ed75fd69b89459090efcb0ee2e
LV2_MINOR_VERSION=8

dnl we don't try to map our version numbers to lv2, instead we increment lv2 minor
dnl version for every single release, and keep micro version at zero, number
dnl must be even to indicate stable version
dnl
dnl see also: https://lv2plug.in/ns/lv2core#minorVersion

AC_SUBST(LV2_MINOR_VERSION)

CLAP_PLUGIN_VERSION=`echo $PACKAGE_VERSION | sed s/-beta/Beta/g;sed s/-alpha/Alpha/g`
dnl from clap SDK: https://github.com/free-audio/clap/blob/main/include/clap/plugin.h
dnl version is an arbitrary string which describes a plugin,
dnl it is useful for the host to understand and be able to compare two different
dnl version strings, so here is a regex like expression which is likely to be
dnl understood by most hosts: MAJOR(.MINOR(.REVISION)?)?( (Alpha|Beta) XREV)?
AC_DEFINE_UNQUOTED(CLAP_PLUGIN_VERSION, ["$CLAP_PLUGIN_VERSION"], [CLAP plugin version])

AC_CONFIG_SRCDIR([src/smenc.cc])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])
saved_CFLAGS="$CFLAGS" ; saved_CXXFLAGS="$CFLAGS" # the AC_PROG_C* macros mess up CFLAGS and CXXFLAGS
AC_PROG_CXX
CFLAGS="$saved_CFLAGS" ; CXXFLAGS="$saved_CXXFLAGS"
LT_INIT
AC_PROG_OBJC
AC_PROG_OBJCXX

dnl
dnl Define package requirements.
dnl

dnl
dnl libglib
dnl
AC_DEFUN([AC_GLIB_REQUIREMENTS],
[
    dnl We need GLib with friends
    PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.0.6, [:],[
      AC_MSG_ERROR([You need to install libglib-2.0 >= 2.0.6 to build this package.

Debian users: aptitude install libglib2.0-dev
])
    ])
    AC_SUBST(GLIB_CFLAGS)
    AC_SUBST(GLIB_LIBS)
])
dnl
dnl libqt5
dnl
AC_DEFUN([AC_QT_REQUIREMENTS],
[
    PKG_CHECK_MODULES(QT, [Qt5Core >= 5.0.0 Qt5Gui >= 5.0.0 Qt5Widgets >= 5.0.0], [with_qt="yes"], [with_qt="no"])
    if test "x$with_qt" = "xyes"; then
      QTCORE_CFLAGS="-DQT_NO_EMIT $QTCORE_CFLAGS"
      QT_CFLAGS="-DQT_NO_EMIT $QT_CFLAGS"
      AC_SUBST(QTCORE_CFLAGS)
      AC_SUBST(QTCORE_LIBS)
      AC_SUBST(QT_CFLAGS)
      AC_SUBST(QT_LIBS)
      MOC=`pkg-config --variable host_bins Qt5Core`/moc
      AC_SUBST(MOC)
    fi
])
dnl
dnl libao
dnl
AC_DEFUN([AC_AO_REQUIREMENTS],
[
    PKG_CHECK_MODULES(AO, ao,[:],[
      AC_MSG_ERROR([It is recommended to install libao to build this package.
You can disable libao using the --without-ao configure option.

Debian users: aptitude install libao-dev
])
    ])
    AC_SUBST(AO_CFLAGS)
    AC_SUBST(AO_LIBS)
])
dnl
dnl freetype
dnl
AC_DEFUN([AC_FREETYPE_REQUIREMENTS],
[
    PKG_CHECK_MODULES(FREETYPE, freetype2)
])
dnl
dnl libcairo
dnl
AC_DEFUN([AC_CAIRO_REQUIREMENTS],
[
    PKG_CHECK_MODULES(CAIRO, cairo,[:],[
      AC_MSG_ERROR([You need to install libcairo to build this package.

Debian users: aptitude install libcairo-dev
])
    ])
    AC_SUBST(CAIRO_CFLAGS)
    AC_SUBST(CAIRO_LIBS)
])
dnl
dnl pkg-config
dnl
AC_DEFUN([AC_PKG_CONFIG_REQUIREMENTS],
[
    AC_CHECK_PROG(HAVE_PKG_CONFIG,pkg-config,yes,no)
    if test "$HAVE_PKG_CONFIG" = "no"; then
      AC_MSG_ERROR([You need to have pkg-config installed to build this package.

Debian users: aptitude install pkg-config
])
    fi
])
dnl
dnl JACK
dnl
AC_DEFUN([AC_JACK_REQUIREMENTS],
[
    dnl disable jack if not found using pkg-config
    PKG_CHECK_MODULES(JACK, [jack], [with_jack="yes"], [with_jack="no"])
])
dnl
dnl sndfile
dnl
AC_DEFUN([AC_SNDFILE_REQUIREMENTS],
[
    PKG_CHECK_MODULES(SNDFILE, [sndfile])
    AC_SUBST(SNDFILE_CFLAGS)
    AC_SUBST(SNDFILE_LIBS)
])
dnl
dnl LV2
dnl
AC_DEFUN([AC_LV2_REQUIREMENTS],
[
  PKG_CHECK_MODULES([LV2], [[lv2 >= 1.2.0]], [with_lv2="yes"], [with_lv2="no"])
])

AC_DEFUN([AC_ZLIB_REQUIREMENTS],
[
  PKG_CHECK_MODULES(ZLIB, zlib)
])

AC_DEFUN([AC_GL_REQUIREMENTS],
[
  PKG_CHECK_MODULES(GL, gl)
])

dnl FFTW3
AC_DEFUN([AC_FFTW_CHECK],
[
    dnl this used to be optional, but is currently required
    PKG_CHECK_MODULES(FFTW, [fftw3f])

    SPECTMORPH_HAVE_FFTW=1
    if test $SPECTMORPH_HAVE_FFTW -gt 0; then
      fftw_save_CFLAGS="$CFLAGS"
      fftw_save_LDFLAGS="$LDFLAGS"
      CFLAGS="$CFLAGS $FFTW_CFLAGS"
      LDFLAGS="$LDFLAGS $FFTW_LIBS"
      AC_MSG_CHECKING([whether FFTW is recent enough])
        AC_COMPILE_IFELSE([
          AC_LANG_SOURCE([
            #include "fftw3.h"
            int x = FFTW_WISDOM_ONLY;
          ])
        ],[
          AC_MSG_RESULT([yes])
        ],[
          AC_MSG_RESULT([no])
          SPECTMORPH_HAVE_FFTW=0
        ])
      AC_MSG_CHECKING([whether FFTW has threadsafe planner API])
        AC_COMPILE_IFELSE([
          AC_LANG_SOURCE([
            #include "fftw3.h"
            void *p = fftwf_make_planner_thread_safe;
          ])
        ],[
          AC_MSG_RESULT([yes])
          SPECTMORPH_HAVE_FFTW_THREADSAFE=1
        ],[
          AC_MSG_RESULT([no])
          SPECTMORPH_HAVE_FFTW_THREADSAFE=0
        ])
      dnl do we need extra libs for fftwf_make_planner_thread_safe? (not on all platforms)
      AC_CHECK_LIB([fftw3f_threads], [fftwf_make_planner_thread_safe], [
        FFTW_LIBS="-lfftw3f_threads $FFTW_LIBS"
      ])
      CFLAGS="$fftw_save_CFLAGS"
      LDFLAGS="$fftw_save_LDFLAGS"
    fi
    AC_DEFINE_UNQUOTED(SPECTMORPH_HAVE_FFTW, $SPECTMORPH_HAVE_FFTW, [Whether libfftw3 is available])
    AC_DEFINE_UNQUOTED(SPECTMORPH_HAVE_FFTW_THREADSAFE, $SPECTMORPH_HAVE_FFTW_THREADSAFE, [Whether fftw3 supports threadsafe planner api])
])

AC_DEFUN([AC_MAN_REQUIREMENTS],
[
  AC_PATH_PROGS(WIKIHTML2MAN, wikihtml2man wikihtml2man.py, false)
  AC_PATH_PROG(GROFF, groff, false)
  IFMAN=
  IFAGING=
  if test x"$enable_maintainer_mode" = xyes ; then
    test x"$WIKIHTML2MAN" = xfalse && {
      AC_MSG_ERROR(This package requires the wikihtml2man.py tool for maintainer builds)
    }
    echo "" | $GROFF -mandoc -Thtml 2>/dev/null | grep -Fq HTML || {
      AC_MSG_ERROR(Failed to test-build HTML manual page via groff)
    }
  else
    IFMAN="#"
    IFAGING="#"
  fi
  AC_SUBST(IFMAN)
  AC_SUBST(IFAGING)
])

dnl ----------------------------- from acbirnet.m4 -------------------------------
dnl Check whether cc accepts a certain option
dnl MC_PROG_CC_SUPPORTS_OPTION(OPTIONS, ACTION-IF-FOUND [,ACTION-IF-NOT-FOUND])
AC_DEFUN([MC_PROG_CC_SUPPORTS_OPTION], [
AC_MSG_CHECKING([whether ${CC-cc} supports $1])
echo >conftest.c;
if ${CC-cc} [$1] -c $CFLAGS conftest.c >/dev/null 2>&1 ; then
    AC_MSG_RESULT(yes)
    [$2]
else
    AC_MSG_RESULT(no)
    [$3]
fi
rm -fr conftest*
])dnl

# expand some installation paths for config.h
#
# backup old vars
bak_datadir="$datadir"
bak_bindir="$bindir"
bak_prefix="$prefix"
bak_exec_prefix="$exec_prefix"
# no prefix given to configure -> use default prefix
test "x$prefix" = xNONE && prefix="$ac_default_prefix"
test "x$exec_prefix" = xNONE && exec_prefix="$prefix"
# define data/bin directory in config.h
datadir=`eval echo "${datadir}"`
bindir=`eval echo "${bindir}"`
AC_DEFINE_UNQUOTED(CONFIGURE_INSTALLPATH_PKGDATADIR, ["`eval echo ${datadir}/spectmorph`"], [Package data directory])
AC_DEFINE_UNQUOTED(CONFIGURE_INSTALLPATH_BINDIR, ["`eval echo ${bindir}`"], [Binary directory])
# restore old vars
datadir="$bak_datadir"
bindir="$bak_bindir"
prefix="$bak_prefix"
exec_prefix="$bak_exec_prefix"

# Less cluttered build output
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Check requirement sets
AC_GLIB_REQUIREMENTS

dnl -------------------- lv2 support is optional ----------------------------
AC_ARG_WITH([lv2], [AS_HELP_STRING([--with-lv2], [build support for LV2])])
if test "x$with_lv2" != "xno"; then
  AC_LV2_REQUIREMENTS
fi
AM_CONDITIONAL([COND_WITH_LV2], [test "x$with_lv2" != "xno"])
dnl -------------------------------------------------------------------------

dnl -------------------- jack support is optional ---------------------------
AC_ARG_WITH([jack], [AS_HELP_STRING([--with-jack], [build support for JACK])])
if test "x$with_jack" != "xno"; then
  AC_JACK_REQUIREMENTS
fi
AM_CONDITIONAL([COND_WITH_JACK], [test "x$with_jack" != "xno"])
dnl -------------------------------------------------------------------------

dnl -------------- qt support is optional -----------------------------------
AC_ARG_WITH([qt], [AS_HELP_STRING([--with-qt], [build tools which need Qt5])])
if test "x$with_qt" != "xno"; then
  AC_QT_REQUIREMENTS
fi
AM_CONDITIONAL([COND_WITH_QT], [test "x$with_qt" != "xno"])
dnl -------------------------------------------------------------------------

dnl -------------- libao support is optional --------------------------------
AC_ARG_WITH([ao], [AS_HELP_STRING([--with-ao], [build tools which need libao])])
if test "x$with_ao" != "xno"; then
  dnl this check will abort if libao is not found
  AC_AO_REQUIREMENTS
  with_ao=yes
fi
AM_CONDITIONAL([COND_WITH_AO], [test "x$with_ao" != "xno"])
if test "x$with_ao" != "xno"; then
  SPECTMORPH_HAVE_AO=1
else
  SPECTMORPH_HAVE_AO=0
fi
AC_DEFINE_UNQUOTED(SPECTMORPH_HAVE_AO, $SPECTMORPH_HAVE_AO, [Whether libao is available])
dnl -------------------------------------------------------------------------

dnl -------------------------------------------------------------------------
AC_ARG_WITH([fonts], [AS_HELP_STRING([--with-fonts], [include fonts in installation])],,with_fonts=no)
AM_CONDITIONAL([COND_WITH_FONTS], [test "x$with_fonts" != "xno"])
dnl -------------------------------------------------------------------------

dnl ----------- link c++ libs statically if requested -----------------------
AC_ARG_WITH([static-cxx], AS_HELP_STRING([--with-static-cxx], [link libstdc++ statically]))

if test "x$with_static_cxx" = "xyes"; then
  STATIC_CXX_LDFLAGS="-static-libstdc++ -static-libgcc"
else
  STATIC_CXX_LDFLAGS=""
fi
AC_SUBST(STATIC_CXX_LDFLAGS)
dnl -------------------------------------------------------------------------

dnl detect host os
AC_CANONICAL_HOST

build_linux=no
build_windows=no
build_macos=no

# Detect the target system
case "${host_os}" in
    linux*)
        build_linux=yes
        ;;
    cygwin*|mingw*)
        build_windows=yes
        ;;
    darwin*)
        build_macos=yes
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac

# Pass the conditionals to automake
AM_CONDITIONAL([COND_LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([COND_WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([COND_MACOS], [test "$build_macos" = "yes"])

if test "$build_linux" = "yes"; then
  PLUGIN_EXT=so
  LV2_UI_TYPE=X11UI
  SHA1SUM=sha1sum
  AC_GL_REQUIREMENTS
fi
if test "$build_windows" = "yes"; then
  PLUGIN_EXT=dll
  LV2_UI_TYPE=WindowsUI
  SHA1SUM=sha1sum
fi
if test "$build_macos" = "yes"; then
  PLUGIN_EXT=dylib
  LV2_UI_TYPE=CocoaUI
  SHA1SUM=shasum
fi
AC_SUBST(PLUGIN_EXT)
AC_SUBST(LV2_UI_TYPE)

AC_FREETYPE_REQUIREMENTS
AC_CAIRO_REQUIREMENTS
AC_SNDFILE_REQUIREMENTS
AC_MAN_REQUIREMENTS
AC_FFTW_CHECK
AC_ZLIB_REQUIREMENTS
AM_ICONV

AM_CFLAGS=""
AM_CXXFLAGS=""

# need c++17 mode
AX_CXX_COMPILE_STDCXX(17, ext)

# == Debug flags ==
AC_ARG_ENABLE(debug, [  --enable-debug=[no/min/yes] turn on debugging, default=minimum],,enable_debug=min)
case "_$enable_debug" in
  _yes)
    debug_flags="-DG_ENABLE_DEBUG -ggdb3 -fverbose-asm -fno-omit-frame-pointer" ;;
  _min*)
    debug_flags="-DG_ENABLE_DEBUG -g1" ;;
  _*)
    debug_flags="-DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS" ;; # -DG_DISABLE_ASSERT
esac
MC_PROG_CC_SUPPORTS_OPTION([$debug_flags], [
  AM_CFLAGS="$AM_CFLAGS $debug_flags"
  AM_CXXFLAGS="$AM_CXXFLAGS $debug_flags"
])
MC_PROG_CC_SUPPORTS_OPTION([-O3], [
  AM_CFLAGS="$AM_CFLAGS -O3"
  AM_CXXFLAGS="$AM_CXXFLAGS -O3"
])
MC_PROG_CC_SUPPORTS_OPTION([-Wall], [
  AM_CFLAGS="$AM_CFLAGS -Wall"
  AM_CXXFLAGS="$AM_CXXFLAGS -Wall"
])
MC_PROG_CC_SUPPORTS_OPTION([-fPIC], [
  AM_CFLAGS="$AM_CFLAGS -fPIC"
  AM_CXXFLAGS="$AM_CXXFLAGS -fPIC"
])
MC_PROG_CC_SUPPORTS_OPTION([-pthread], [
  AM_CFLAGS="$AM_CFLAGS -pthread"
  AM_CXXFLAGS="$AM_CXXFLAGS -pthread"
])
MC_PROG_CC_SUPPORTS_OPTION([-ffast-math], [
  AM_CFLAGS="$AM_CFLAGS -ffast-math"
  AM_CXXFLAGS="$AM_CXXFLAGS -ffast-math"
])

if test "x$build_cpu" != "xaarch64"; then
  gcc_target_sse="-msse -msse2 -msse3 -mmmx"
  MC_PROG_CC_SUPPORTS_OPTION([$gcc_target_sse],     [ AM_CXXFLAGS="$AM_CXXFLAGS $gcc_target_sse" ])
fi

gcc_optimize_extra="-funroll-loops -ftree-vectorize -finline-functions -ftracer -ftree-loop-distribution -ftree-loop-ivcanon -ftree-loop-im -minline-all-stringops"
MC_PROG_CC_SUPPORTS_OPTION([$gcc_optimize_extra], [ AM_CXXFLAGS="$AM_CXXFLAGS $gcc_optimize_extra" ])

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)
# --- AC_SUBST(AM_CPPFLAGS) ---

# check if compiler supports -fvisibility=hidden
gl_VISIBILITY

AC_ARG_ENABLE(oprofile,AS_HELP_STRING([--enable-oprofile], [setup compiler flags to allow oprofile usage]),
[
MC_PROG_CC_SUPPORTS_OPTION([-fno-omit-frame-pointer], [
  # CFLAGS="$CFLAGS -fno-omit-frame-pointer"
  # CXXFLAGS="$CXXFLAGS -fno-omit-frame-pointer"
])
])

AC_ARG_ENABLE(debug-cxx,AS_HELP_STRING([--enable-debug-cxx], [setup compiler flags to do C++ STL debug checks]),
[
  CXXFLAGS="$CXXFLAGS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC"
])

asan_flags_ok="no"
AC_ARG_ENABLE(asan,AS_HELP_STRING([--enable-asan], [enable address sanitizer]),
[
asan_flags="-fno-omit-frame-pointer -fstack-protector-all -fno-inline -g -DG_ENABLE_DEBUG -fsanitize=address -fno-common"
MC_PROG_CC_SUPPORTS_OPTION([$asan_flags], [
  AM_CFLAGS="$AM_CFLAGS $asan_flags"
  AM_CXXFLAGS="$AM_CXXFLAGS $asan_flags"
  asan_flags_ok="yes"
])
])

dnl -------------------- undefined behaviour sanitizer ----------------------
AC_ARG_ENABLE([ubsan], [AS_HELP_STRING([--enable-ubsan], [build using undefined behaviour sanitizer])],
[
  CXXFLAGS="$CXXFLAGS -fsanitize=undefined -fno-omit-frame-pointer -fstack-protector-all -fno-inline"
  if test "$build_macos" = "no"; then
    LDFLAGS="$LDFLAGS -lubsan"
  fi
],
[
  enable_ubsan=no
])
dnl -------------------------------------------------------------------------

dnl -------------------------------------------------------------------------
INSTRUMENTS_ARCHIVE=`cd $srcdir && echo $PWD`/data/spectmorph-instruments-$VERSION.tar.xz
AC_SUBST(INSTRUMENTS_ARCHIVE)

# instrument downloader
AC_ARG_WITH([download-instruments], [AS_HELP_STRING([--with-download-instruments], [download instrument archive])],,with_download_instruments=no)
if test "x$with_download_instruments" = "xyes"; then
  # if we already have the correct version, no download is necessary
  if test -f $INSTRUMENTS_ARCHIVE; then
    ACTUAL_SHA1SUM=$($SHA1SUM $INSTRUMENTS_ARCHIVE | awk '{print $1}')
    if test "x$ACTUAL_SHA1SUM" != "x$INST_SHA1SUM"; then
      rm $INSTRUMENTS_ARCHIVE
    fi
  fi
  if ! test -f $INSTRUMENTS_ARCHIVE; then
    echo
    AC_MSG_NOTICE([downloading instruments ($INST_TAG)...])
    cd data
    DOWNLOAD_URL=https://github.com/swesterfeld/spectmorph-instruments/releases/download/$INST_TAG/spectmorph-instruments-$PACKAGE_VERSION.tar.xz;
    if ! wget -q $DOWNLOAD_URL; then
      AC_MSG_ERROR([Failed to download: $DOWNLOAD_URL])
    fi
    cd ..
    echo
  fi
fi

# instrument checksum verification
if test -f $INSTRUMENTS_ARCHIVE; then
  AC_MSG_CHECKING([if instruments checksum is correct])
  ACTUAL_SHA1SUM=$($SHA1SUM $INSTRUMENTS_ARCHIVE | awk '{print $1}')
  if test "x$ACTUAL_SHA1SUM" != "x$INST_SHA1SUM"; then
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([SHA-1 checksum verification of $INSTRUMENTS_ARCHIVE failed!])
  else
    AC_MSG_RESULT([yes])
  fi
fi

dnl -------------------------------------------------------------------------

AC_CONFIG_FILES([Makefile
3rdparty/Makefile 3rdparty/minizip/Makefile 3rdparty/hiir/Makefile 3rdparty/clap/Makefile
lib/Makefile src/Makefile tools/Makefile tests/Makefile docs/Makefile clap/Makefile
glui/Makefile glui/pugl/Makefile jack/Makefile inspector/Makefile lv2/Makefile vst/Makefile
vst/vestige/Makefile data/Makefile data/templates/Makefile data/fonts/Makefile instedit/Makefile
tools/sminstbuilder spectmorph.pc spectmorphglui.pc lv2/spectmorph.ttl
lv2/manifest.ttl windows/config.sh macos/config.sh macos/Info.plist
static/config.sh static/install.sh
])
AC_CONFIG_FILES([tests/test-common.sh])
AC_OUTPUT

# Output summary message

echo
echo "---------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "---------------------------------------------------------------------------"
echo "  LV2 plug-in support  . .: $with_lv2"
echo "  JACK support . . . . . .: $with_jack"
echo "  Qt5 available  . . . . .: $with_qt"
echo "  libao support  . . . . .: $with_ao"
echo "  Fonts . . . . . . . . . : $with_fonts"

if test ! -f $INSTRUMENTS_ARCHIVE; then
  echo "  Instruments  . . . . . .: missing (use --with-download-instruments to fix this)"
else
  echo "  Instruments  . . . . . .: yes"
fi

if test "x$asan_flags_ok" != "xno"; then
  echo "  Address Sanitizer  . . .: $asan_flags"
fi

if test "x$enable_ubsan" != "xno"; then
  echo "  UB Sanitizer . . . . . .: $enable_ubsan"
fi

echo "  Install prefix . . . . .: $prefix"
echo
echo "Now type 'make', followed by 'make install' as root."
echo
