include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui $(CFLAG_VISIBILITY) -I$(top_srcdir)/3rdparty

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GLUI_LIBS = $(top_builddir)/glui/libspectmorphglui.la

spectmorph_clapdir = $(libdir)/clap

plugindir = $(spectmorph_clapdir)
plugin_LTLIBRARIES = SpectMorph.la

SpectMorph_la_SOURCES = smclapplugin.cc
SpectMorph_la_LIBADD  = $(SPECTMORPH_LIBS) $(SPECTMORPH_GLUI_LIBS)
SpectMorph_la_LDFLAGS = -module -avoid-version -disable-static

EXTRA_DIST += ldscript.map

# we only want to install the .so file; not the .la file
install-data-hook:
	rm -f $(DESTDIR)$(spectmorph_clapdir)/spectmorph_lv2.la
	rm -f $(DESTDIR)$(spectmorph_clapdir)/spectmorph_lv2.a

uninstall-hook:
	rm -f $(DESTDIR)$(spectmorph_clapdir)/spectmorph_lv2.so
