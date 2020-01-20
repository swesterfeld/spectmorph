include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui $(CFLAG_VISIBILITY)

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GLUI_LIBS = $(top_builddir)/glui/libspectmorphglui.la

SMHDRS = smlv2common.hh smlv2plugin.hh smlv2ui.hh

spectmorph_lv2dir = $(libdir)/lv2/spectmorph.lv2

plugindir = $(spectmorph_lv2dir)
plugin_LTLIBRARIES = spectmorph_lv2.la

spectmorph_lv2_la_SOURCES = smlv2plugin.cc smlv2ui.cc $(SMHDRS)
spectmorph_lv2_la_LIBADD  = $(SPECTMORPH_LIBS) $(SPECTMORPH_GLUI_LIBS)
spectmorph_lv2_la_LDFLAGS = -module -avoid-version -disable-static

spectmorph_lv2_DATA = manifest.ttl spectmorph.ttl

CLEANFILES += spectmorph.ttl
spectmorph.ttl: $(top_builddir)/config.status

EXTRA_DIST += manifest.ttl

# we only want to install the .so file; not the .la file
install-data-hook:
	rm -f $(DESTDIR)$(spectmorph_lv2dir)/spectmorph_lv2.la
	rm -f $(DESTDIR)$(spectmorph_lv2dir)/spectmorph_lv2.a

uninstall-hook:
	rm -f $(DESTDIR)$(spectmorph_lv2dir)/spectmorph_lv2.so

spectmorph_lv2.so.static:
	$(CXX) -shared -o spectmorph_lv2.so.static $(srcdir)/smlv2plugin.cc $(srcdir)/smlv2ui.cc $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
		$(top_builddir)/glui/.libs/libspectmorphglui.a \
		$(top_builddir)/glui/pugl/.libs/libspectmorphpugl.a \
		$(top_builddir)/lib/.libs/libspectmorph.a \
		$(STATIC_CXX_LDFLAGS) $(LDFLAGS) \
		$(BSE_LIBS) $(FFTW_LIBS) $(CAIRO_LIBS) $(SNDFILE_LIBS) $(GL_LIBS)
