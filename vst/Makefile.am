include $(top_srcdir)/Makefile.decl

SUBDIRS = vestige

AM_CXXFLAGS = $(BSE_CFLAGS) $(JACK_CFLAGS) $(QT_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/gui

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GUI_LIBS = $(top_builddir)/gui/libspectmorphgui.la

spectmorph_vstdir = $(libdir)/vst

plugindir = $(spectmorph_vstdir)
plugin_LTLIBRARIES = spectmorph_vst.la

SMVSTHEADERS = smvstplugin.hh smvstui.hh smvstcommon.hh
SMVSTSOURCES = smvstplugin.cc smvstui.cc moc_smvstui.cc

spectmorph_vst_la_SOURCES = $(SMVSTSOURCES) $(SMVSTHEADERS)
spectmorph_vst_la_LIBADD  = $(QT_LIBS) $(BSE_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GUI_LIBS)
spectmorph_vst_la_LDFLAGS = -rpath $(spectmorph_vstdir) -avoid-version -module -export-symbols-regex "VSTPluginMain|main"

testlink: testlink.cc
	$(CXX) -o /dev/null testlink.cc $(SMVSTSOURCES) $(AM_CFLAGS) $(AM_CXXFLAGS) $(QT_LIBS) $(BSE_LIBS) -fPIC -L../lib/.libs -lspectmorph -L../gui/.libs -lspectmorphgui

moc_smvstui.cc: smvstui.hh
	$(MOC) smvstui.hh > moc_smvstui.cc

# we only want to install the .so file; not the .la file
install-data-hook:
	rm -f $(DESTDIR)$(spectmorph_vstdir)/spectmorph_vst.la
	rm -f $(DESTDIR)$(spectmorph_vstdir)/spectmorph_vst.a

uninstall-hook:
	rm -f $(DESTDIR)$(spectmorph_vstdir)/spectmorph_vst.so
