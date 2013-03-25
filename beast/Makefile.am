include $(top_srcdir)/Makefile.decl

plugindir=/usr/local/beast/lib/bse/v$(BSE_VERSION)/plugins

AM_CXXFLAGS=$(BSE_CFLAGS) $(QT_CFLAGS) -O3 -I$(top_srcdir)/lib

SUBDIRS = gui

plugin_cxxflags = $(BSE_CFLAGS)
plugin_libs = $(QT_LIBS) $(BSE_LIBS) $(top_builddir)/lib/libspectmorph.la
plugin_ldflags = $(BSE_LDFLAGS) -avoid-version
plugin_LTLIBRARIES =

## SpectMorph OSC
spectmorphosc_la_SOURCES = spectmorphosc.cc
spectmorphosc_la_LDFLAGS = -module $(plugin_ldflags)
spectmorphosc_la_LIBADD  = $(plugin_libs)
plugin_LTLIBRARIES += spectmorphosc.la

## SpectMorph Delay
spectmorphdelay_la_SOURCES = spectmorphdelay.cc
spectmorphdelay_la_LDFLAGS = -module $(plugin_ldflags)
spectmorphdelay_la_LIBADD  = $(plugin_libs)
plugin_LTLIBRARIES += spectmorphdelay.la

EXTRA_DIST += spectmorphosc.idl spectmorphdelay.idl

spectmorphosc.genidl.hh: $(srcdir)/spectmorphosc.idl
	sfidl --plugin $(srcdir)/spectmorphosc.idl > spectmorphosc.genidl.hh

spectmorphdelay.genidl.hh: $(srcdir)/spectmorphdelay.idl
	sfidl --plugin $(srcdir)/spectmorphdelay.idl > spectmorphdelay.genidl.hh

$(srcdir)/spectmorphosc.cc: spectmorphosc.genidl.hh
$(srcdir)/spectmorphdelay.cc: spectmorphdelay.genidl.hh

CLEANFILES += spectmorphosc.genidl.hh spectmorphdelay.genidl.hh
