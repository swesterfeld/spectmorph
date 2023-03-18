include $(top_srcdir)/Makefile.decl

SUBDIRS = vestige

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui $(CFLAG_VISIBILITY)

EXTRA_DIST += macos_export_symbols testlink.cc ldscript.map

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GLUI_LIBS = $(top_builddir)/glui/libspectmorphglui.la

spectmorph_vstdir = $(libdir)/vst

plugindir = $(spectmorph_vstdir)
plugin_LTLIBRARIES = spectmorph_vst.la

SMVSTHEADERS = smvstplugin.hh smvstui.hh smvstcommon.hh smvstresize.hh
SMVSTSOURCES = smvstplugin.cc smvstui.cc

spectmorph_vst_la_SOURCES = $(SMVSTSOURCES) $(SMVSTHEADERS)
spectmorph_vst_la_LIBADD  = $(SPECTMORPH_LIBS) $(SPECTMORPH_GLUI_LIBS)
spectmorph_vst_la_LDFLAGS = -rpath $(spectmorph_vstdir) -avoid-version -module -no-undefined

testlink: testlink.cc
	$(CXX) -o /dev/null testlink.cc $(SMVSTSOURCES) $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(BSE_LIBS) -L../lib/.libs -lspectmorph -L../glui/.libs -lspectmorphglui

# we only want to install the .so file; not the .la file
install-data-hook:
	rm -f $(DESTDIR)$(spectmorph_vstdir)/spectmorph_vst.la
	rm -f $(DESTDIR)$(spectmorph_vstdir)/spectmorph_vst.a

uninstall-hook:
	rm -f $(DESTDIR)$(spectmorph_vstdir)/spectmorph_vst.so

spectmorph_vst.so.static:
	$(CXX) -shared -o spectmorph_vst.so.static $(srcdir)/smvstplugin.cc $(srcdir)/smvstui.cc \
	       -DSM_STATIC_LINUX $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	        $(STATIC_PLUGIN_LIBS) \
		$(STATIC_CXX_LDFLAGS) $(LDFLAGS) \
		$(BSE_LIBS) $(FFTW_LIBS) $(CAIRO_LIBS) $(SNDFILE_LIBS) $(GL_LIBS) \
		-Wl,--version-script=$(srcdir)/ldscript.map

if COND_WINDOWS
install-exec-hook: SpectMorph.dll
	mkdir -p $(libdir)/vst
	cp SpectMorph.dll $(libdir)/vst

SpectMorph.dll: $(plugin_LTLIBRARIES)
	$(CXX) -shared -o SpectMorph.dll $(srcdir)/smvstplugin.cc $(srcdir)/smvstui.cc -static \
		$(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_PLUGIN_LIBS)
endif
if COND_MACOS

SYS_DEPS = -liconv -lm -framework Cocoa -framework OpenGL

install-exec-hook: SpectMorph.so
	mkdir -p $(libdir)/vst
	cp SpectMorph.so $(libdir)/vst

SpectMorph.so: $(plugin_LTLIBRARIES)
	$(CXX) -shared -o SpectMorph.so $(srcdir)/smvstplugin.cc $(srcdir)/smvstui.cc -exported_symbols_list macos_export_symbols -static $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_PLUGIN_LIBS) $(SYS_DEPS)
endif
