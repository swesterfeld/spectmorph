include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

SMHDRS = smspectrumview.hh smmicroconf.hh smspectrumwindow.hh smpixelarray.hh smtimefreqview.hh \
         smzoomcontroller.hh smnavigator.hh smmainwinow.hh smfftparamwindow.hh smfftthread.hh \
         smcommon.hh

SMSRCS = smmicroconf.cc smspectrumview.cc \
         smspectrumwindow.cc smpixelarray.cc smtimefreqview.cc \
         smzoomcontroller.cc smnavigator.cc smmainwindow.cc \
         smfftparamwindow.cc smfftthread.cc

noinst_PROGRAMS = testinspector

bin_PROGRAMS = sminspector

sminspector_SOURCES = sminspector.cc $(SMSRCS) $(SMHDRS)
sminspector_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS)
sminspector_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)

testinspector_SOURCES = testinspector.cc $(SMSRCS) $(SMHDRS)
testinspector_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS)
testinspector_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)
