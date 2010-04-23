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
AC_DEFUN([AC_BSE_REQUIREMENTS],
[
    dnl We need libbse
    PKG_CHECK_MODULES(BSE, bse >= 0.7.2)
    AC_SUBST(BSE_CFLAGS)
    AC_SUBST(BSE_LIBS)
])
AC_DEFUN([AC_GTK_REQUIREMENTS],
[
    dnl We need libbse
    PKG_CHECK_MODULES(GTK, [gtk+-2.0],[:],[
      AC_MSG_ERROR([You need to install libgtk-2.0 to build this package.

Debian users: aptitude install libgtk2.0-dev
])
    ])
    AC_SUBST(GTK_CFLAGS)
    AC_SUBST(GTK_LIBS)
])
AC_DEFUN([AC_AO_REQUIREMENTS],
[
    dnl We need libao
    PKG_CHECK_MODULES(AO, ao,[:],[
      AC_MSG_ERROR([You need to install libao to build this package.

Debian users: aptitude install libao-dev
])
    ])
    AC_SUBST(AO_CFLAGS)
    AC_SUBST(AO_LIBS)
])
AC_DEFUN([AC_LAPACK_REQUIREMENTS],
[
    dnl We need liblapack
    AC_CHECK_LIB(lapack,sgesv_,[
      LAPACK_LIBS="-llapack"
      AC_SUBST(LAPACK_LIBS)
    ],[
      AC_MSG_ERROR([You need to install liblapack to build this package.

Debian users: aptitude install liblapack-dev
])
    ])
])

# We need pkg-config
AC_CHECK_PROG(HAVE_PKG_CONFIG,pkg-config,yes,no)
if test "$HAVE_PKG_CONFIG" == "no"; then
  AC_MSG_ERROR([You need to have pkg-config installed to build this package.

Debian users: aptitude install pkg-config
])
fi

# Check requirement sets
AC_GLIB_REQUIREMENTS
AC_BSE_REQUIREMENTS
AC_AO_REQUIREMENTS
AC_GTK_REQUIREMENTS
AC_LAPACK_REQUIREMENTS
BOOST_REQUIRE([1.34])
BOOST_FIND_HEADER([boost/numeric/bindings/lapack/lapack.hpp],[
  AC_MSG_ERROR([You need to install boost-numeric-bindings to build this package.])
])

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile tools/Makefile python/Makefile tests/Makefile])
AC_OUTPUT
