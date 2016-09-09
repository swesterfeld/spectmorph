include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS = $(BSE_CFLAGS) $(JACK_CFLAGS) $(QT_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/gui

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GUI_LIBS = $(top_builddir)/gui/libspectmorphgui.la

SMHDRS = smlv2common.hh

spectmorph_lv2dir = $(libdir)/lv2/spectmorph.lv2

plugindir = $(spectmorph_lv2dir)
plugin_libs = $(QT_LIBS) $(BSE_LIBS) $(SPECTMORPH_LIBS)
plugin_LTLIBRARIES = spectmorph_lv2.la spectmorph_lv2_ui.la

spectmorph_lv2_la_SOURCES = smlv2plugin.cc $(SMHDRS)
spectmorph_lv2_la_LIBADD  = $(plugin_libs)
spectmorph_lv2_la_LDFLAGS = -module -avoid-version -disable-static -export-symbols-regex "lv2_descriptor"

spectmorph_lv2_ui_la_SOURCES = smlv2ui.cc $(SMHDRS) smlv2ui.hh moc_smlv2ui.cc
spectmorph_lv2_ui_la_LIBADD  = $(plugin_libs) $(SPECTMORPH_GUI_LIBS)
spectmorph_lv2_ui_la_LDFLAGS = -module -avoid-version -disable-static -export-symbols-regex "lv2_descriptor"

dist_spectmorph_lv2_DATA = manifest.ttl spectmorph.ttl

moc_smlv2ui.cc: smlv2ui.hh
	$(MOC) smlv2ui.hh > moc_smlv2ui.cc
