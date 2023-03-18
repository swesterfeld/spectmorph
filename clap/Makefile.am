include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui $(CFLAG_VISIBILITY) -I$(top_srcdir)/3rdparty

spectmorph_clapdir = $(libdir)/clap

CLEANFILES += SpectMorph.clap
EXTRA_DIST += smclapplugin.cc ldscript.map

if COND_LINUX
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
endif

if COND_WINDOWS
all-local: SpectMorph.clap

install-exec-hook: SpectMorph.clap
	mkdir -p $(spectmorph_clapdir)
	cp SpectMorph.clap $(spectmorph_clapdir)

SpectMorph.clap: $(srcdir)/smclapplugin.cc
	$(CXX) -shared -o SpectMorph.clap -I$(top_builddir) $(srcdir)/smclapplugin.cc -static \
		$(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_PLUGIN_SM_LIBS) $(STATIC_PLUGIN_LIBS)
endif

if COND_MACOS

STATIC_DEPS_DIR = $(top_builddir)/macos/prefix
STATIC_DEPS = \
  $(STATIC_DEPS_DIR)/lib/libsndfile.a \
  $(STATIC_DEPS_DIR)/lib/libvorbisenc.a \
  $(STATIC_DEPS_DIR)/lib/libvorbis.a \
  $(STATIC_DEPS_DIR)/lib/libogg.a \
  $(STATIC_DEPS_DIR)/lib/libFLAC.a \
  $(STATIC_DEPS_DIR)/lib/libcairo.a \
  $(STATIC_DEPS_DIR)/lib/libpixman-1.a \
  $(STATIC_DEPS_DIR)/lib/libglib-2.0.a \
  $(STATIC_DEPS_DIR)/lib/libintl.a \
  $(STATIC_DEPS_DIR)/lib/libfftw3f.a \
  $(STATIC_DEPS_DIR)/lib/libfreetype.a \
  $(STATIC_DEPS_DIR)/lib/libpng16.a \
  $(STATIC_DEPS_DIR)/lib/libz.a

SYS_DEPS = -liconv -lm -framework Cocoa -framework OpenGL

install-exec-hook: SpectMorph.so
	mkdir -p $(libdir)/clap
	cp SpectMorph.so $(libdir)/clap

SpectMorph.so: $(plugin_LTLIBRARIES)
	$(CXX) -shared -o SpectMorph.so -I$(top_builddir) $(srcdir)/smclapplugin.cc -exported_symbols_list macos_export_symbols -static $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_PLUGIN_SM_LIBS) $(STATIC_DEPS) $(SYS_DEPS)
endif
