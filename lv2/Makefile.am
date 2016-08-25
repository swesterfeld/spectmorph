include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS = $(BSE_CFLAGS) $(JACK_CFLAGS) $(GTKMM_CFLAGS) $(QT_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/gui

LV2_LIBDIR=/usr/lib
spectmorph_lv2dir = $(LV2_LIBDIR)/lv2/spectmorph.lv2

plugindir = $(spectmorph_lv2dir)
plugin_libs = $(QT_LIBS) $(BSE_LIBS) $(top_builddir)/lib/libspectmorph.la
plugin_LTLIBRARIES = spectmorph_lv2.la

spectmorph_lv2_la_SOURCES = spectmorph_lv2.cc
spectmorph_lv2_la_LIBADD  = $(plugin_libs)
spectmorph_lv2_la_LDFLAGS = -module -avoid-version -disable-static -export-symbols-regex "lv2_descriptor"

dist_spectmorph_lv2_DATA = manifest.ttl spectmorph.ttl
