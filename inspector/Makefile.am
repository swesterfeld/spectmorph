include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

SMHDRS = smspectrumview.hh smspectrumwindow.hh smpixelarray.hh smtimefreqview.hh \
         smzoomcontroller.hh smnavigator.hh smmainwindow.hh smfftparamwindow.hh smfftthread.hh \
         smcommon.hh smcwt.hh

SMSRCS = smspectrumview.cc \
         smspectrumwindow.cc smpixelarray.cc smtimefreqview.cc \
         smzoomcontroller.cc smnavigator.cc smmainwindow.cc \
         smfftparamwindow.cc smfftthread.cc smcwt.cc

noinst_PROGRAMS = testinspector
noinst_LTLIBRARIES = libsminspector.la

bin_PROGRAMS = sminspector

libsminspector_la_SOURCES = $(SMSRCS) $(SMHDRS)
libsminspector_la_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)

sminspector_SOURCES = sminspector.cc
sminspector_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS) libsminspector.la
sminspector_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)

testinspector_SOURCES = testinspector.cc
testinspector_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS) libsminspector.la
testinspector_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)
