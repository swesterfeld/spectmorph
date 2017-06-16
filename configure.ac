AC_INIT([spectmorph], [0.3.2])
AC_CONFIG_SRCDIR([src/smenc.cc])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([foreign])
saved_CFLAGS="$CFLAGS" ; saved_CXXFLAGS="$CFLAGS" # the AC_PROG_C* macros mess up CFLAGS and CXXFLAGS
AC_PROG_CXX
CFLAGS="$saved_CFLAGS" ; CXXFLAGS="$saved_CXXFLAGS"
AC_PROG_LIBTOOL

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
    dnl check for libbse unless --without-beast was used
    if test "x$with_beast" != "xno"; then
        PKG_CHECK_MODULES(BSE, [[bse >= 0.9]], [with_beast="yes"], [with_beast="no"])
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
dnl libqt4 is not supported in any way; things will break if you try to build against Qt4
dnl
AC_DEFUN([AC_QT4_REQUIREMENTS],
[
    PKG_CHECK_MODULES(QT, [QtCore >= 4.0.0 QtGui >= 4.0.0],[:],[
      AC_MSG_ERROR([You need to install Qt4 to build this package.])
    ])
    QT_CFLAGS="-DQT_NO_EMIT $QT_CFLAGS"
    AC_SUBST(QT_CFLAGS)
    AC_SUBST(QT_LIBS)
    MOC=`pkg-config --variable moc_location QtCore`
    AC_SUBST(MOC)
])
dnl
dnl libqt5
dnl
AC_DEFUN([AC_QT_REQUIREMENTS],
[
    PKG_CHECK_MODULES(QT, [Qt5Core >= 5.0.0 Qt5Gui >= 5.0.0 Qt5Widgets >= 5.0.0],[:],[
      AC_MSG_ERROR([You need to install Qt5 to build this package.])
    ])
    QT_CFLAGS="-DQT_NO_EMIT $QT_CFLAGS"
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
    dnl We need JACK libs and cflags
    PKG_CHECK_MODULES(JACK, [jack])
    AC_SUBST(JACK_CFLAGS)
    AC_SUBST(JACK_LIBS)
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

dnl FFTW3
AC_DEFUN([AC_FFTW_CHECK],
[
    dnl this used to be optional, but is currently required
    PKG_CHECK_MODULES(FFTW, [fftw3 fftw3f])

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

# define pkgdatadir
# backup old vars
bak_datadir="$datadir"
bak_prefix="$prefix"
bak_exec_prefix="$exec_prefix"
# no prefix given to configure -> use default prefix
test "x$prefix" = xNONE && prefix="$ac_default_prefix"
test "x$exec_prefix" = xNONE && exec_prefix="$prefix"
# define data directory in config.h
datadir=`eval echo "${datadir}"`
AC_DEFINE_UNQUOTED(CONFIGURE_INSTALLPATH_PKGDATADIR, ["`eval echo ${datadir}/spectmorph`"], [Package data directory])
# restore old vars
datadir="$bak_datadir"
prefix="$bak_prefix"
exec_prefix="$bak_exec_prefix"

# Less cluttered build output
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Check requirement sets
AC_GLIB_REQUIREMENTS

dnl -------------------- libbse dependency can be disabled -------------------
AC_ARG_WITH([beast], AS_HELP_STRING([--with-beast], [Build SpectMorph without depending on BEAST]))

AC_BSE_REQUIREMENTS

dnl -------------------------------------------------------------------------

dnl -------------------- lv2 support is optional ----------------------------
AC_ARG_WITH([lv2], [AS_HELP_STRING([--with-lv2], [build support for LV2])])
if test "$with_lv2" != "xno"; then
  AC_LV2_REQUIREMENTS
fi
AM_CONDITIONAL([COND_WITH_LV2], [test "x$with_lv2" != "xno"])
dnl -------------------------------------------------------------------------

# need boost basic linear algebra for LPC
BOOST_REQUIRE
BOOST_NUMERIC_UBLAS

AC_AO_REQUIREMENTS
AC_QT_REQUIREMENTS
AC_JACK_REQUIREMENTS
AC_SNDFILE_REQUIREMENTS
AC_MAN_REQUIREMENTS
AC_FFTW_CHECK

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
lib/Makefile gui/Makefile src/Makefile tools/Makefile python/Makefile tests/Makefile docs/Makefile
beast/Makefile jack/Makefile inspector/Makefile beast/gui/Makefile lv2/Makefile vst/Makefile
vst/vestige/Makefile data/Makefile data/templates/Makefile
tools/sminstbuilder spectmorph.pc spectmorphgui.pc
])
AC_CONFIG_FILES([tests/test-common.sh])
AC_OUTPUT

# Output summary message

echo
echo "---------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "---------------------------------------------------------------------------"
echo "  LV2 plug-in support  . .: $with_lv2"
echo "  BEAST plug-in support  .: $with_beast"
echo "  Install prefix . . . . .: $prefix"
echo
echo "Now type 'make', followed by 'make install' as root."
echo
