AC_INIT([spectmorph], [0.3.0])
AC_CONFIG_SRCDIR([src/smenc.cc])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AM_PATH_PYTHON
AM_CHECK_PYTHON_HEADERS([:],[
  AC_MSG_ERROR([You need to install python development headers to build this package.

Debian users: aptitude install python-dev
])
])
AC_PROG_CXX
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
    PKG_CHECK_MODULES(BSE, bse >= 0.9)
    AC_SUBST(BSE_CFLAGS)
    AC_SUBST(BSE_LIBS)
    BSE_VERSION=`pkg-config --modversion bse`
    AC_SUBST(BSE_VERSION)
    bseplugindir=`pkg-config --variable=plugindir bse`
    AC_SUBST(bseplugindir)
])
dnl
dnl libnobse (used in case libbse is not present/wanted)
dnl
AC_DEFUN([AC_NOBSE_REQUIREMENTS],
[
    BSE_CFLAGS='-I$(top_srcdir)/nobse '$GLIB_CFLAGS
    AC_SUBST(BSE_CFLAGS)
    BSE_LIBS='$(top_srcdir)/nobse/libspectmorph_nobse.la '$GLIB_LIBS
    AC_SUBST(BSE_LIBS)
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
dnl LV2
dnl
AC_DEFUN([AC_LV2_REQUIREMENTS],
[
  PKG_CHECK_MODULES([LV2], [[lv2 >= 1.2.0]], [with_lv2="yes"], [with_lv2="no"])
])

dnl FFTW3 (optional requirement)
AC_DEFUN([AC_FFTW_CHECK],
[
    PKG_CHECK_MODULES(FFTW, [fftw3 fftw3f],[
       SPECTMORPH_HAVE_FFTW=1
    ],[
       SPECTMORPH_HAVE_FFTW=0
    ])
    if test $SPECTMORPH_HAVE_FFTW -gt 0; then
    fftw_save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $FFTW_CFLAGS"
    AC_MSG_CHECKING([whether FFTW is recent enough])
      AC_COMPILE_IFELSE([
        #include "fftw3.h"
        int x = FFTW_WISDOM_ONLY;
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
bak_datadir="$datadir"
datadir=`eval echo "${datadir}"`
AC_DEFINE_UNQUOTED(CONFIGURE_INSTALLPATH_PKGDATADIR, ["`eval echo ${datadir}/spectmorph`"], [Package data directory])
datadir="$bak_datadir"

# Less cluttered build output
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Check requirement sets
AC_GLIB_REQUIREMENTS

dnl -------------------- libbse dependency can be disabled -------------------
AC_ARG_WITH([beast],
    AS_HELP_STRING([--without-beast], [Build SpectMorph without depending on BEAST]))

# build with beast dependency unless explicitely disabled
AM_CONDITIONAL([COND_WITH_BEAST], [test "$with_beast" != no])

if test "x$with_beast" = "xno"; then
  AC_NOBSE_REQUIREMENTS
else
  AC_BSE_REQUIREMENTS
fi
dnl -------------------------------------------------------------------------

dnl -------------------- lv2 support is optional ----------------------------
AC_ARG_WITH([lv2], [AS_HELP_STRING([--with-lv2], [build support for LV2])])
AM_CONDITIONAL([COND_WITH_LV2], [test "x$with_lv2" != "xno"])
if test "$with_lv2" != "xno"; then
  AC_LV2_REQUIREMENTS
fi
dnl -------------------------------------------------------------------------

AC_AO_REQUIREMENTS
AC_QT_REQUIREMENTS
AC_JACK_REQUIREMENTS
AC_MAN_REQUIREMENTS
AC_FFTW_CHECK
MC_PROG_CC_SUPPORTS_OPTION([-Wall], [
  CFLAGS="$CFLAGS -Wall"
  CXXFLAGS="$CXXFLAGS -Wall"
])
MC_PROG_CC_SUPPORTS_OPTION([-fPIC], [
  CFLAGS="$CFLAGS -fPIC"
  CXXFLAGS="$CXXFLAGS -fPIC"
])
MC_PROG_CC_SUPPORTS_OPTION([-std=gnu++0x], [
  CXXFLAGS="$CXXFLAGS -std=gnu++0x"
])


MC_PROG_CC_SUPPORTS_OPTION([-msse -mmmx -ftree-vectorize], [
  SSE_FLAGS="-msse -mmmx -ftree-vectorize"
],[
  SSE_FLAGS=""
])
AC_SUBST(SSE_FLAGS)

AC_ARG_ENABLE(oprofile,AS_HELP_STRING([--enable-oprofile], [setup compiler flags to allow oprofile usage]),
[
MC_PROG_CC_SUPPORTS_OPTION([-fno-omit-frame-pointer], [
  CFLAGS="$CFLAGS -fno-omit-frame-pointer"
  CXXFLAGS="$CXXFLAGS -fno-omit-frame-pointer"
])
])

AC_ARG_ENABLE(debug-cxx,AS_HELP_STRING([--enable-debug-cxx], [setup compiler flags to do C++ STL debug checks]),
[
  CFLAGS="$CFLAGS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC"
  CXXFLAGS="$CXXFLAGS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC"
])

AC_CONFIG_FILES([Makefile
nobse/Makefile nobse/birnet/Makefile nobse/bse/Makefile
lib/Makefile gui/Makefile src/Makefile tools/Makefile python/Makefile tests/Makefile docs/Makefile
beast/Makefile jack/Makefile inspector/Makefile beast/gui/Makefile lv2/Makefile vst/Makefile
vst/vestige/Makefile data/Makefile data/templates/Makefile
tools/sminstbuilder spectmorph.pc
])
AC_CONFIG_FILES([tests/test-common.sh])
AC_OUTPUT
