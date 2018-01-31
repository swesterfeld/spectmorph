if COND_LINUX
  PUGL_SOURCE = pugl_x11.c
endif
if COND_MACOS
  PUGL_SOURCE = pugl_osx.m
endif

noinst_LTLIBRARIES = libspectmorphpugl.la

libspectmorphpugl_la_SOURCES = $(PUGL_SOURCE)
libspectmorphpugl_la_CFLAGS = $(AM_CFLAGS) -I.. -DPUGL_HAVE_CAIRO -DPUGL_HAVE_GL
libspectmorphpugl_la_OBJCFLAGS = $(AM_OBJCFLAGS) -I.. -DPUGL_HAVE_CAIRO -DPUGL_HAVE_GL
libspectmorphpugl_la_LIBTOOLFLAGS = --tag CC
