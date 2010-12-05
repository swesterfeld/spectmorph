include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

SMHDRS = smspectrumview.hh smmicroconf.hh smspectrumwindow.hh

noinst_PROGRAMS = sminspector

sminspector_SOURCES = sminspector.cc smmicroconf.cc smspectrumview.cc \
                      smspectrumwindow.cc $(SMHDRS)
sminspector_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS)
sminspector_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)
