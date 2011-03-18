include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

SMHDRS = smzoomcontroller.hh
SMSRCS = smzoomcontroller.cc

lib_LTLIBRARIES = libspectmorphgui.la

libspectmorphgui_la_SOURCES = $(SMSRCS) $(SMHDRS)
libspectmorphgui_la_CXXFLAGS = $(GTKMM_CFLAGS) $(AM_CXXFLAGS)
libspectmorphgui_la_LIBADD = $(BSE_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS)
