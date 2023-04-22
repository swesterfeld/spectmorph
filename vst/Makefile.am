include $(top_srcdir)/Makefile.decl

SUBDIRS = vestige

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui $(CFLAG_VISIBILITY)

EXTRA_DIST += macos_export_symbols testlink.cc ldscript.map \
  smvstplugin.hh smvstui.hh smvstcommon.hh smvstresize.hh \
  smvstplugin.cc smvstui.cc

spectmorph_vstdir = $(libdir)/vst

plugindir = $(spectmorph_vstdir)

if COND_LINUX
# need to keep compatibility for hosts which may rely on the binary name
PLUGIN = spectmorph_vst.so
else
PLUGIN = SpectMorph.$(PLUGIN_EXT)
endif

all-local: $(PLUGIN)

install-exec-hook: $(PLUGIN)
	mkdir -p $(DESTDIR)$(spectmorph_vstdir)
	cp $(PLUGIN) $(DESTDIR)$(spectmorph_vstdir)

uninstall-hook:
	rm -f $(DESTDIR)$(spectmorph_vstdir)/$(PLUGIN)

CLEANFILES += $(PLUGIN) spectmorph_vst.so.static

PLUGIN_SRCS = $(srcdir)/smvstplugin.cc $(srcdir)/smvstui.cc
PLUGIN_DEPS = $(PLUGIN_SRCS) \
	$(srcdir)/smvstcommon.hh $(srcdir)/smvstplugin.hh $(srcdir)/smvstui.hh $(srcdir)/smvstresize.hh \
	$(top_builddir)/lib/libspectmorph.la $(top_builddir)/glui/libspectmorphglui.la

if COND_LINUX
spectmorph_vst.so: $(PLUGIN_DEPS)
	$(CXX) -fPIC -DPIC -shared -o spectmorph_vst.so -I$(top_builddir) $(PLUGIN_SRCS) \
        $(CXXFLAGS) $(AM_CXXFLAGS) \
	$(top_builddir)/lib/.libs/libspectmorph.so \
	$(top_builddir)/glui/.libs/libspectmorphglui.so \
	$(STATIC_CXX_LDFLAGS) $(LDFLAGS) \
	-Wl,-rpath=$(libdir) -Wl,--version-script=$(srcdir)/ldscript.map

spectmorph_vst.so.static: $(PLUGIN_DEPS)
	$(CXX) -shared -o spectmorph_vst.so.static $(srcdir)/smvstplugin.cc $(srcdir)/smvstui.cc \
	       -DSM_STATIC_LINUX $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
		$(STATIC_CXX_LDFLAGS) $(LDFLAGS) $(STATIC_PLUGIN_LIBS) \
		-Wl,--version-script=$(srcdir)/ldscript.map
endif
if COND_WINDOWS
SpectMorph.dll: $(PLUGIN_DEPS)
	$(CXX) -shared -o SpectMorph.dll $(srcdir)/smvstplugin.cc $(srcdir)/smvstui.cc -static \
		$(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_PLUGIN_LIBS)
	$(STRIP) SpectMorph.dll
endif
if COND_MACOS
SpectMorph.dylib: $(PLUGIN_DEPS)
	$(CXX) -shared -o SpectMorph.dylib $(srcdir)/smvstplugin.cc $(srcdir)/smvstui.cc -exported_symbols_list macos_export_symbols -static \
	       $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_PLUGIN_LIBS)
endif
