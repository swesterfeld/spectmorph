include $(top_srcdir)/Makefile.decl

SUBDIRS = pugl

if COND_LINUX
  PUGL_LIBS = pugl/libspectmorphpugl.la -lX11 -lGL
endif

AM_CXXFLAGS += $(BSE_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src -DPUGL_HAVE_GL -DPUGL_HAVE_CAIRO

smuitest_SOURCES = smuitest.cc
smuitest_LDADD = $(BSE_LIBS) $(CAIRO_LIBS) $(PUGL_LIBS)

noinst_PROGRAMS = smuitest
