if COND_LINUX
  PUGL_SOURCE = pugl_x11.c
endif
if COND_MACOS
  PUGL_SOURCE = pugl_osx.m
endif
if COND_WINDOWS
  PUGL_SOURCE = pugl_win.cpp
endif

SMHDRS = cairo_gl.h glew.h gl.h glu.h pugl.h pugl_internal.h

noinst_LTLIBRARIES = libspectmorphpugl.la

PUGL_CFLAGS = $(CAIRO_CFLAGS) -I$(top_srcdir)/glui -DPUGL_HAVE_CAIRO -DPUGL_HAVE_GL -DGL_SILENCE_DEPRECATION $(CFLAG_VISIBILITY)

libspectmorphpugl_la_SOURCES = $(PUGL_SOURCE) $(SMHDRS)
libspectmorphpugl_la_CFLAGS = $(AM_CFLAGS) $(PUGL_CFLAGS)
libspectmorphpugl_la_OBJCFLAGS = $(AM_OBJCFLAGS) $(PUGL_CFLAGS)
libspectmorphpugl_la_CXXFLAGS = $(AM_CXXFLAGS) $(PUGL_CFLAGS)
libspectmorphpugl_la_LIBTOOLFLAGS = --tag CC

smpuglincludedir = $(includedir)/spectmorph/smglui/pugl
smpuglinclude_HEADERS = $(SMHDRS)
