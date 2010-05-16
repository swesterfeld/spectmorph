AC_INIT([spectmorph], [0.0.2])
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
    PKG_CHECK_MODULES(BSE, bse >= 0.7.2)
    AC_SUBST(BSE_CFLAGS)
    AC_SUBST(BSE_LIBS)
])
dnl
dnl libgtk
dnl
AC_DEFUN([AC_GTK_REQUIREMENTS],
[
    PKG_CHECK_MODULES(GTK, [gtk+-2.0],[:],[
      AC_MSG_ERROR([You need to install libgtk-2.0 to build this package.

Debian users: aptitude install libgtk2.0-dev
])
    ])
    AC_SUBST(GTK_CFLAGS)
    AC_SUBST(GTK_LIBS)
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
dnl liblapack
dnl
AC_DEFUN([AC_LAPACK_REQUIREMENTS],
[
    AC_CHECK_LIB(lapack,sgesv_,[
      LAPACK_LIBS="-llapack"
      AC_SUBST(LAPACK_LIBS)
    ],[
      AC_MSG_ERROR([You need to install liblapack to build this package.

Debian users: aptitude install liblapack-dev
])
    ])
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
dnl boost-numeric-bindings
dnl
AC_DEFUN([AC_BOOST_NUMERIC_BINDINGS_REQUIREMENTS],
[
    AC_ARG_WITH(boost-numeric-bindings, [AC_HELP_STRING([--with-boost-numeric-bindings=PREFIX],[where boost-numeric-bindings is installed @<:@$PREFIX@:>@])],
                                        BOOST_NUMERIC_BINDINGS_PREFIX="$withval", BOOST_NUMERIC_BINDINGS_PREFIX="$prefix")
    if test "$BOOST_NUMERIC_BINDINGS_PREFIX" = "NONE"
    then
        BOOST_NUMERIC_BINDINGS_PREFIX=$ac_default_prefix
    fi

    BOOST_NUMERIC_BINDINGS_CPPFLAGS="-I$BOOST_NUMERIC_BINDINGS_PREFIX/include/boost-numeric-bindings"
    bnb_save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $BOOST_NUMERIC_BINDINGS_CPPFLAGS"
    BOOST_FIND_HEADER([boost/numeric/bindings/lapack/lapack.hpp],[
      AC_MSG_ERROR([You need to install boost-numeric-bindings to build this package.

    To download it, see: http://mathema.tician.de/software/boost-numeric-bindings

If you install boost-numeric-bindings in a prefix other than the prefix you install this
package in, you may need to use:

    configure --with-boost-numeric-bindings=/path/to/bnb-prefix/
])
    ])
    CPPFLAGS="$bnb_save_CPPFLAGS"
    AC_SUBST([BOOST_NUMERIC_BINDINGS_PREFIX])
    AC_SUBST([BOOST_NUMERIC_BINDINGS_CPPFLAGS])
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



# Check requirement sets
AC_GLIB_REQUIREMENTS
AC_BSE_REQUIREMENTS
AC_AO_REQUIREMENTS
AC_GTK_REQUIREMENTS
AC_LAPACK_REQUIREMENTS
BOOST_REQUIRE([1.34])
AC_BOOST_NUMERIC_BINDINGS_REQUIREMENTS
MC_PROG_CC_SUPPORTS_OPTION([-msse -mmmx -ftree-vectorize], [
  SSE_FLAGS="-msse -mmmx -ftree-vectorize"
],[
  SSE_FLAGS=""
])
AC_SUBST(SSE_FLAGS)

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile tools/Makefile python/Makefile tests/Makefile docs/Makefile])
AC_CONFIG_FILES([tests/smresvalue.sh], [chmod +x tests/smresvalue.sh])
AC_OUTPUT
