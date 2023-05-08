include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui $(CFLAG_VISIBILITY) -I$(top_srcdir)/3rdparty

spectmorph_clapdir = $(libdir)/clap

CLEANFILES += SpectMorph.clap SpectMorph.clap.static
EXTRA_DIST += smclapplugin.cc ldscript.map macos_export_symbols

all-local: SpectMorph.clap

install-exec-hook: SpectMorph.clap
	mkdir -p $(DESTDIR)$(spectmorph_clapdir)
	cp SpectMorph.clap $(DESTDIR)$(spectmorph_clapdir)

uninstall-hook:
	rm -f $(DESTDIR)$(spectmorph_clapdir)/SpectMorph.clap

PLUGIN_DEPS = $(srcdir)/smclapplugin.cc $(top_builddir)/lib/libspectmorph.la $(top_builddir)/glui/libspectmorphglui.la

if COND_LINUX
SpectMorph.clap: $(PLUGIN_DEPS)
	$(CXX) -fPIC -DPIC -shared -o SpectMorph.clap -I$(top_builddir) $(srcdir)/smclapplugin.cc $(CXXFLAGS) $(AM_CXXFLAGS) \
	$(top_builddir)/lib/.libs/libspectmorph.so \
	$(top_builddir)/glui/.libs/libspectmorphglui.so \
	$(STATIC_CXX_LDFLAGS) $(LDFLAGS) \
	-Wl,-rpath=$(libdir) -Wl,--version-script=$(srcdir)/ldscript.map

SpectMorph.clap.static: $(PLUGIN_DEPS)
	$(CXX) -shared -o SpectMorph.clap.static -I$(top_builddir) $(srcdir)/smclapplugin.cc \
	       -DSM_STATIC_LINUX $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
		$(STATIC_CXX_LDFLAGS) $(LDFLAGS) $(STATIC_PLUGIN_LIBS) \
		-Wl,--version-script=$(srcdir)/ldscript.map
endif

if COND_WINDOWS
SpectMorph.clap: $(PLUGIN_DEPS)
	$(CXX) -shared -o SpectMorph.clap -I$(top_builddir) $(srcdir)/smclapplugin.cc -static \
		$(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_PLUGIN_LIBS)
	$(STRIP) SpectMorph.clap
endif

if COND_MACOS
SpectMorph.clap: $(PLUGIN_DEPS)
	$(CXX) -shared -o SpectMorph.clap -I$(top_builddir) $(srcdir)/smclapplugin.cc -exported_symbols_list $(srcdir)/macos_export_symbols -static \
		$(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_PLUGIN_LIBS)
endif
