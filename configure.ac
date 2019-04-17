AC_INIT([spectmorph], [0.4.1])
AC_CONFIG_SRCDIR([src/smenc.cc])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([foreign])
saved_CFLAGS="$CFLAGS" ; saved_CXXFLAGS="$CFLAGS" # the AC_PROG_C* macros mess up CFLAGS and CXXFLAGS
AC_PROG_CXX
CFLAGS="$saved_CFLAGS" ; CXXFLAGS="$saved_CXXFLAGS"
AC_PROG_LIBTOOL
AC_PROG_OBJC
AC_PROG_OBJCXX

dnl Version splitted

[MAJOR_VERSION=`echo $PACKAGE_VERSION | sed 's/\([0-9]\+\)\.\([0-9]\+\)\.\([0-9]\+\)\(.*\)/\1/'`]
[MINOR_VERSION=`echo $PACKAGE_VERSION | sed 's/\([0-9]\+\)\.\([0-9]\+\)\.\([0-9]\+\)\(.*\)/\2/'`]
[MICRO_VERSION=`echo $PACKAGE_VERSION | sed 's/\([0-9]\+\)\.\([0-9]\+\)\.\([0-9]\+\)\(.*\)/\3/'`]

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(MICRO_VERSION)

dnl
dnl Define package requirements.
dnl

dnl
dnl libglib
dnl
AC_DEFUN([AC_GLIB_REQUIREMENTS],
[
    dnl We need GLib with friends
    PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.0.6 gobject-2.0 gmodule-2.0 gthread-2.0, [:],[
      AC_MSG_ERROR([You need to install libglib-2.0 >= 2.0.6 to build this package.

Debian users: aptitude install libglib2.0-dev
])
    ])
    AC_SUBST(GLIB_CFLAGS)
    AC_SUBST(GLIB_LIBS)
])
dnl
dnl libbse
dnl
AC_DEFUN([AC_BSE_REQUIREMENTS],
[
    dnl check for libbse only if --with-beast was used
    if test "x$with_beast" = "xyes"; then
        PKG_CHECK_MODULES(BSE, [[bse >= 0.9]])
    else
        with_beast=no
    fi
    if test "x$with_beast" = "xno"; then
        SPECTMORPH_HAVE_BSE=0

        BSE_CFLAGS="$GLIB_CFLAGS"
        BSE_LIBS="$GLIB_LIBS"
    else
        SPECTMORPH_HAVE_BSE=1

        BSE_VERSION=`pkg-config --modversion bse`
        AC_SUBST(BSE_VERSION)
        bseplugindir=`pkg-config --variable=plugindir bse`
        AC_SUBST(bseplugindir)
    fi
    AC_SUBST(BSE_CFLAGS)
    AC_SUBST(BSE_LIBS)
    AC_DEFINE_UNQUOTED(SPECTMORPH_HAVE_BSE, $SPECTMORPH_HAVE_BSE, [Whether libbse is available])
    AM_CONDITIONAL([COND_WITH_BEAST], [test "$with_beast" != no])
])
dnl
dnl libqt5
dnl
AC_DEFUN([AC_QT_REQUIREMENTS],
[
    PKG_CHECK_MODULES(QTCORE, [Qt5Core >= 5.0.0],[:],[
      AC_MSG_ERROR([You need to install Qt5 to build this package.

You can manually disable Qt5 using --without-qt (in this case some tools will not be built).
      ])
    ])
    PKG_CHECK_MODULES(QT, [Qt5Core >= 5.0.0 Qt5Gui >= 5.0.0 Qt5Widgets >= 5.0.0],[:],[
      AC_MSG_ERROR([You need to install Qt5 to build this package.])
    ])
    QTCORE_CFLAGS="-DQT_NO_EMIT $QTCORE_CFLAGS"
    QT_CFLAGS="-DQT_NO_EMIT $QT_CFLAGS"
    AC_SUBST(QTCORE_CFLAGS)
    AC_SUBST(QTCORE_LIBS)
    AC_SUBST(QT_CFLAGS)
    AC_SUBST(QT_LIBS)
    MOC=`pkg-config --variable host_bins Qt5Core`/moc
    AC_SUBST(MOC)
])
dnl
dnl libao
dnl
AC_DEFUN([AC_AO_REQUIREMENTS],
[
    PKG_CHECK_MODULES(AO, ao,[:],[
      AC_MSG_ERROR([You need to install libao to build this package.

Debian users: aptitude install libao-dev
])
    ])
    AC_SUBST(AO_CFLAGS)
    AC_SUBST(AO_LIBS)
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

dnl FFTW3
AC_DEFUN([AC_FFTW_CHECK],
[
    dnl this used to be optional, but is currently required
    PKG_CHECK_MODULES(FFTW, [fftw3f])

    SPECTMORPH_HAVE_FFTW=1
    if test $SPECTMORPH_HAVE_FFTW -gt 0; then
    fftw_save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $FFTW_CFLAGS"
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
    fi
    CFLAGS="$fftw_save_CFLAGS"
    AC_DEFINE_UNQUOTED(SPECTMORPH_HAVE_FFTW, $SPECTMORPH_HAVE_FFTW, [Whether libfftw3 is available])
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

dnl -------------------- libbse dependency can be disabled -------------------
AC_ARG_WITH([beast], AS_HELP_STRING([--with-beast], [developer only: build against libbse]))

AC_BSE_REQUIREMENTS

dnl -------------------------------------------------------------------------

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

dnl -------------- qt support can be disabled manually ----------------------
AC_ARG_WITH([qt], [AS_HELP_STRING([--with-qt], [build tools which need Qt5])])
if test "x$with_qt" != "xno"; then
  dnl this check will abort if Qt5 is not found
  AC_QT_REQUIREMENTS
  with_qt=yes
fi
AM_CONDITIONAL([COND_WITH_QT], [test "x$with_qt" != "xno"])
dnl -------------------------------------------------------------------------

dnl allow setting location of the instruments tarball (default in data/ dir)
AC_ARG_WITH([instruments], [AS_HELP_STRING([--with-instruments=FILE], [use custom instruments tarball])],,with_instruments=no)

if test "x$with_instruments" != "xno"; then
  # translate relative path to absolute path
  with_instruments_dir=`dirname $with_instruments`
  with_instruments_file=`basename $with_instruments`

  INSTRUMENTS_ARCHIVE=`cd $with_instruments_dir && echo $PWD`/$with_instruments_file
  if test ! -f $INSTRUMENTS_ARCHIVE; then
    AC_MSG_ERROR([invalid instrument tarball path: $with_instruments])
  fi
else
  INSTRUMENTS_ARCHIVE=`cd $srcdir && echo $PWD`/data/spectmorph-instruments-$VERSION.tar.xz
fi
AC_SUBST(INSTRUMENTS_ARCHIVE)
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

AC_AO_REQUIREMENTS
AC_CAIRO_REQUIREMENTS
AC_SNDFILE_REQUIREMENTS
AC_MAN_REQUIREMENTS
AC_FFTW_CHECK
AC_ZLIB_REQUIREMENTS

AM_CFLAGS=""
AM_CXXFLAGS=""

# need c++11 mode
AX_CXX_COMPILE_STDCXX_11(ext)

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

gcc_target_sse="-msse -msse2 -msse3 -mmmx"
MC_PROG_CC_SUPPORTS_OPTION([$gcc_target_sse],     [ AM_CXXFLAGS="$AM_CXXFLAGS $gcc_target_sse" ])

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
  # CFLAGS="$CFLAGS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC"
  # CXXFLAGS="$CXXFLAGS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC"
])

AC_CONFIG_FILES([Makefile
3rdparty/Makefile 3rdparty/minizip/Makefile
lib/Makefile gui/Makefile src/Makefile tools/Makefile tests/Makefile docs/Makefile
glui/Makefile glui/pugl/Makefile jack/Makefile inspector/Makefile vst/Makefile
vst/vestige/Makefile data/Makefile data/templates/Makefile instedit/Makefile
tools/sminstbuilder spectmorph.pc spectmorphgui.pc spectmorphglui.pc lv2/spectmorph.ttl
windows/config.sh macos/config.sh macos/Info.plist
])
# lv2/Makefile <- disabled due to new features which need instance access
AC_CONFIG_FILES([tests/test-common.sh])
AC_OUTPUT

# Output summary message

echo
echo "---------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "---------------------------------------------------------------------------"
echo "  LV2 plug-in support  . .: $with_lv2"
echo "  JACK support   . . . . .: $with_jack"
echo "  Qt5 available  . . . . .: $with_qt"

if test "x$with_instruments" != "xno"; then
  echo "  Instruments  . . . . . .: $INSTRUMENTS_ARCHIVE"
fi

echo "  Install prefix . . . . .: $prefix"

if test "x$with_beast" = "xyes"; then
  echo
  echo "  * building against libbse (this is a developer only option, not recommended)"
fi
echo
echo "Now type 'make', followed by 'make install' as root."
echo
