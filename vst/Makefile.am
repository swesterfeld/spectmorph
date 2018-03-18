include $(top_srcdir)/Makefile.decl

SUBDIRS = vestige

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GLUI_LIBS = $(top_builddir)/glui/libspectmorphglui.la

spectmorph_vstdir = $(libdir)/vst

plugindir = $(spectmorph_vstdir)
plugin_LTLIBRARIES = spectmorph_vst.la

SMVSTHEADERS = smvstplugin.hh smvstui.hh smvstcommon.hh
SMVSTSOURCES = smvstplugin.cc smvstui.cc

spectmorph_vst_la_SOURCES = $(SMVSTSOURCES) $(SMVSTHEADERS)
spectmorph_vst_la_LIBADD  = $(BSE_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GLUI_LIBS)
spectmorph_vst_la_LDFLAGS = -rpath $(spectmorph_vstdir) -avoid-version -module -export-symbols-regex "VSTPluginMain|main"

testlink: testlink.cc
	$(CXX) -o /dev/null testlink.cc $(SMVSTSOURCES) $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(QT_LIBS) $(BSE_LIBS) -fPIC -L../lib/.libs -lspectmorph -L../gui/.libs -lspectmorphgui

# we only want to install the .so file; not the .la file
install-data-hook:
	rm -f $(DESTDIR)$(spectmorph_vstdir)/spectmorph_vst.la
	rm -f $(DESTDIR)$(spectmorph_vstdir)/spectmorph_vst.a

uninstall-hook:
	rm -f $(DESTDIR)$(spectmorph_vstdir)/spectmorph_vst.so

if COND_WINDOWS
STATIC_DLL_LIBS = \
  $(top_builddir)/glui/.libs/libspectmorphglui.a \
  $(top_builddir)/glui/pugl/.libs/libspectmorphpugl.a \
  $(top_builddir)/lib/.libs/libspectmorph.a

install-exec-hook: SpectMorph.dll
	mkdir -p $(libdir)/vst
	cp SpectMorph.dll $(libdir)/vst

SpectMorph.dll: $(plugin_LTLIBRARIES)
	$(CXX) -shared -o SpectMorph.dll $(srcdir)/smvstplugin.cc $(srcdir)/smvstui.cc -static $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_DLL_LIBS) $(BSE_LIBS) $(FFTW_LIBS) $(CAIRO_LIBS) -lopengl32 -lgdi32 -luser32 -lcomdlg32 -liconv
endif
