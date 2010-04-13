AC_INIT([spectmorph], [0.0.2])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/smenc.cc])
AC_PROG_CXX
AC_PROG_LIBTOOL

dnl
dnl Define package requirements.
dnl
AC_DEFUN([AC_GLIB_REQUIREMENTS],
[
    dnl We need GLib with friends
    PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.0.6 gobject-2.0 gmodule-2.0 gthread-2.0)
    AC_SUBST(GLIB_CFLAGS)
    AC_SUBST(GLIB_LIBS)
])
AC_DEFUN([AC_BSE_REQUIREMENTS],
[
    dnl We need libbse
    PKG_CHECK_MODULES(BSE, bse)
    AC_SUBST(BSE_CFLAGS)
    AC_SUBST(BSE_LIBS)
])
AC_DEFUN([AC_GTK_REQUIREMENTS],
[
    dnl We need libbse
    PKG_CHECK_MODULES(GTK, [gtk+-2.0])
    AC_SUBST(GTK_CFLAGS)
    AC_SUBST(GTK_LIBS)
])
AC_DEFUN([AC_AO_REQUIREMENTS],
[
    dnl We need libao
    PKG_CHECK_MODULES(AO, ao)
    AC_SUBST(AO_CFLAGS)
    AC_SUBST(AO_LIBS)
])
AC_DEFUN([AC_LAPACK_REQUIREMENTS],
[
    dnl We need liblapack
    AC_CHECK_LIB(lapack,sgesv_)
])


# Check requirement sets
AC_GLIB_REQUIREMENTS
AC_BSE_REQUIREMENTS
AC_AO_REQUIREMENTS
AC_GTK_REQUIREMENTS
AC_LAPACK_REQUIREMENTS

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile tools/Makefile])
AC_OUTPUT
