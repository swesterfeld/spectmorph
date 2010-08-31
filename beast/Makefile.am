include $(top_srcdir)/Makefile.decl

plugindir=/usr/local/beast/lib/bse/v0.7.2/plugins

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 -D_BIRNET_SOURCE_EXTENSIONS -I$(top_srcdir)/lib

plugin_cxxflags = $(BSE_CFLAGS)
plugin_libs = $(BSE_LIBS) $(top_builddir)/lib/libspectmorph.la
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
