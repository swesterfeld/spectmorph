include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui $(CFLAG_VISIBILITY) -I$(top_srcdir)/3rdparty

spectmorph_clapdir = $(libdir)/clap

CLEANFILES += SpectMorph.clap
EXTRA_DIST += smclapplugin.cc ldscript.map

all-local: SpectMorph.clap

install-exec-hook: SpectMorph.clap
	mkdir -p $(DESTDIR)$(spectmorph_clapdir)
	cp SpectMorph.clap $(DESTDIR)$(spectmorph_clapdir)

uninstall-hook:
	rm -f $(DESTDIR)$(spectmorph_clapdir)/SpectMorph.clap

SpectMorph.clap: $(srcdir)/smclapplugin.cc $(top_builddir)/lib/libspectmorph.la $(top_builddir)/glui/libspectmorphglui.la
	$(CXX) -fPIC -DPIC -shared -o SpectMorph.clap -I$(top_builddir) $(srcdir)/smclapplugin.cc $(CXXFLAGS) $(AM_CXXFLAGS) \
	$(top_builddir)/lib/.libs/libspectmorph.so \
	$(top_builddir)/glui/.libs/libspectmorphglui.so \
	$(STATIC_CXX_LDFLAGS) $(LDFLAGS) $(SNDFILE_LIBS) \
	-Wl,-rpath=$(libdir) -Wl,--version-script=$(srcdir)/ldscript.map
