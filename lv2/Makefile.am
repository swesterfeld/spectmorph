include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(LV2_CFLAGS) $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui $(CFLAG_VISIBILITY)

spectmorph_lv2dir = $(libdir)/lv2/spectmorph.lv2

plugindir = $(spectmorph_lv2dir)

spectmorph_lv2_DATA = manifest.ttl spectmorph.ttl

CLEANFILES += spectmorph_lv2.$(PLUGIN_EXT) spectmorph.ttl spectmorph_lv2.so.static
spectmorph.ttl: $(top_builddir)/config.status

EXTRA_DIST += smlv2common.hh smlv2plugin.cc smlv2plugin.hh smlv2ui.cc smlv2ui.hh ldscript.map

all-local: spectmorph_lv2.$(PLUGIN_EXT)

install-exec-hook: spectmorph_lv2.$(PLUGIN_EXT)
	mkdir -p $(DESTDIR)$(spectmorph_lv2dir)
	cp spectmorph_lv2.$(PLUGIN_EXT) $(DESTDIR)$(spectmorph_lv2dir)

uninstall-hook:
	rm -f $(DESTDIR)$(spectmorph_lv2dir)/spectmorph_lv2.$(PLUGIN_EXT)

PLUGIN_SRCS = $(srcdir)/smlv2plugin.cc $(srcdir)/smlv2ui.cc
PLUGIN_DEPS = $(PLUGIN_SRCS) \
	$(srcdir)/smlv2common.hh $(srcdir)/smlv2plugin.hh $(srcdir)/smlv2ui.hh
	$(top_builddir)/lib/libspectmorph.la $(top_builddir)/glui/libspectmorphglui.la

if COND_LINUX
spectmorph_lv2.so: $(PLUGIN_DEPS)
	$(CXX) -fPIC -DPIC -shared -o spectmorph_lv2.so -I$(top_builddir) $(PLUGIN_SRCS) \
        $(CXXFLAGS) $(AM_CXXFLAGS) \
	$(top_builddir)/lib/.libs/libspectmorph.so \
	$(top_builddir)/glui/.libs/libspectmorphglui.so \
	$(STATIC_CXX_LDFLAGS) $(LDFLAGS) \
	-Wl,-rpath=$(libdir) -Wl,--version-script=$(srcdir)/ldscript.map

spectmorph_lv2.so.static: $(PLUGIN_DEPS)
	$(CXX) -shared -o spectmorph_lv2.so.static $(PLUGIN_SRCS) \
	        -DSM_STATIC_LINUX $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
		$(STATIC_CXX_LDFLAGS) $(LDFLAGS) $(STATIC_PLUGIN_LIBS) \
		-Wl,--version-script=$(srcdir)/ldscript.map
endif

if COND_WINDOWS
spectmorph_lv2.dll: $(PLUGIN_DEPS)
	$(CXX) -shared -o spectmorph_lv2.dll -I$(top_builddir) $(PLUGIN_SRCS) -static \
		$(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_PLUGIN_LIBS)
	$(STRIP) spectmorph_lv2.dll
endif

if COND_MACOS
spectmorph_lv2.dylib: $(PLUGIN_DEPS)
	$(CXX) -shared -o spectmorph_lv2.dylib -I$(top_builddir) $(PLUGIN_SRCS) -exported_symbols_list macos_export_symbols -static \
		$(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_PLUGIN_LIBS)
endif
