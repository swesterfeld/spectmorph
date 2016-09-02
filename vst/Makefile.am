include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS = $(BSE_CFLAGS) $(JACK_CFLAGS) $(QT_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/gui

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GUI_LIBS = $(top_builddir)/gui/libspectmorphgui.la

spectmorph_vstdir = /home/stefan/.vst

plugindir = $(spectmorph_vstdir)
plugin_libs = $(QT_LIBS) $(BSE_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GUI_LIBS)
plugin_LTLIBRARIES = spectmorph_vst.la

spectmorph_vst_la_SOURCES = smvstplugin.cc
spectmorph_vst_la_LIBADD  = $(plugin_libs)
spectmorph_vst_la_LDFLAGS = -module -avoid-version -disable-static
