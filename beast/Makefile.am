include $(top_srcdir)/Makefile.decl

plugindir=$(bseplugindir)

AM_CXXFLAGS += $(BSE_CFLAGS) $(QTCORE_CFLAGS) -I$(top_srcdir)/lib

SUBDIRS = gui

plugin_cxxflags = $(BSE_CFLAGS)
plugin_libs = $(QTCORE_LIBS) $(BSE_LIBS) $(top_builddir)/lib/libspectmorph.la
plugin_ldflags = $(BSE_LDFLAGS) -avoid-version
plugin_LTLIBRARIES =

## SpectMorph OSC
spectmorphosc_la_SOURCES = spectmorphosc.cc
spectmorphosc_la_LDFLAGS = -module $(plugin_ldflags)
spectmorphosc_la_LIBADD  = $(plugin_libs)
plugin_LTLIBRARIES += spectmorphosc.la

EXTRA_DIST += spectmorphosc.idl

spectmorphosc.genidl.hh: $(srcdir)/spectmorphosc.idl
	sfidl --plugin $(srcdir)/spectmorphosc.idl > spectmorphosc.genidl.hh

$(srcdir)/spectmorphosc.cc: spectmorphosc.genidl.hh

CLEANFILES += spectmorphosc.genidl.hh
