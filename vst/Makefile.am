include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS = $(BSE_CFLAGS) $(JACK_CFLAGS) $(QT_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/gui

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GUI_LIBS = $(top_builddir)/gui/libspectmorphgui.la

spectmorph_vstdir = /home/stefan/.vst

plugindir = $(spectmorph_vstdir)
plugin_libs = $(QT_LIBS) $(BSE_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GUI_LIBS)
plugin_LTLIBRARIES = spectmorph_vst.la

SMVSTSOURCES = smvstplugin.cc smvstui.cc moc_smvstui.cc

spectmorph_vst_la_SOURCES = $(SMVSTSOURCES)
spectmorph_vst_la_LIBADD  = $(plugin_libs)
spectmorph_vst_la_LDFLAGS = -module -avoid-version -disable-static

testlink: testlink.cc
	$(CXX) -o /dev/null testlink.cc $(SMVSTSOURCES) $(AM_CFLAGS) $(AM_CXXFLAGS) $(QT_LIBS) $(BSE_LIBS) -fPIC -L../lib/.libs -lspectmorph -L../gui/.libs -lspectmorphgui

moc_smvstui.cc: smvstui.hh
	moc smvstui.hh > moc_smvstui.cc
