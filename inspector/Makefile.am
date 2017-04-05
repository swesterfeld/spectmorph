include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(QT_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src -I$(top_srcdir)/gui \
             -I$(top_srcdir)/jack

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la $(top_builddir)/gui/libspectmorphgui.la
SPECTMORPH_JACK_LIBS = $(top_builddir)/jack/libspectmorphjack.la

SMHDRS = smspectrumview.hh smspectrumwindow.hh smpixelarray.hh smtimefreqview.hh \
         smnavigator.hh smtimefreqwindow.hh smfftparamwindow.hh smfftthread.hh \
         smcommon.hh smcwt.hh smsamplewindow.hh smplayerwindow.hh smlpcwindow.hh \
         smlpcview.hh smdisplayparamwindow.hh smnavigatorwindow.hh smsamplewinview.hh \
         smtimefreqwinview.hh

SMSRCS = smspectrumview.cc \
         smspectrumwindow.cc smpixelarray.cc smtimefreqview.cc \
         smnavigator.cc smtimefreqwindow.cc \
         smfftparamwindow.cc smfftthread.cc smcwt.cc smsamplewindow.cc \
         smplayerwindow.cc smlpcwindow.cc smlpcview.cc smdisplayparamwindow.cc smnavigatorwindow.cc \
         smsamplewinview.cc smtimefreqwinview.cc \
         moc_smnavigator.cc moc_smsamplewindow.cc moc_smsamplewinview.cc moc_smplayerwindow.cc \
         moc_smtimefreqwindow.cc moc_smfftparamwindow.cc moc_smfftthread.cc moc_smcwt.cc \
         moc_smtimefreqview.cc moc_smspectrumwindow.cc moc_smspectrumview.cc moc_smdisplayparamwindow.cc \
         moc_smlpcview.cc moc_smlpcwindow.cc moc_smnavigatorwindow.cc moc_smtimefreqwinview.cc

noinst_PROGRAMS = testinspector
noinst_LTLIBRARIES = libsminspector.la

bin_PROGRAMS = sminspector

libsminspector_la_SOURCES = $(SMSRCS) $(SMHDRS)
libsminspector_la_CXXFLAGS = $(AM_CXXFLAGS)

sminspector_SOURCES = sminspector.cc
sminspector_LDADD = libsminspector.la $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_JACK_LIBS) $(QT_LIBS)
sminspector_CXXFLAGS = $(AM_CXXFLAGS)

testinspector_SOURCES = testinspector.cc
testinspector_LDADD = libsminspector.la $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_JACK_LIBS) $(QT_LIBS)
testinspector_CXXFLAGS = $(AM_CXXFLAGS)

moc_smnavigator.cc: smnavigator.hh
	$(MOC) smnavigator.hh > moc_smnavigator.cc

moc_smsamplewinview.cc: smsamplewinview.hh
	$(MOC) smsamplewinview.hh > moc_smsamplewinview.cc

moc_smsamplewindow.cc: smsamplewindow.hh
	$(MOC) smsamplewindow.hh > moc_smsamplewindow.cc

moc_smplayerwindow.cc: smplayerwindow.hh
	$(MOC) smplayerwindow.hh > moc_smplayerwindow.cc

moc_smtimefreqwindow.cc: smtimefreqwindow.hh
	$(MOC) smtimefreqwindow.hh > moc_smtimefreqwindow.cc

moc_smfftparamwindow.cc: smfftparamwindow.hh
	$(MOC) smfftparamwindow.hh > moc_smfftparamwindow.cc

moc_smfftthread.cc: smfftthread.hh
	$(MOC) smfftthread.hh > moc_smfftthread.cc

moc_smcwt.cc: smcwt.hh
	$(MOC) smcwt.hh > moc_smcwt.cc

moc_smtimefreqview.cc: smtimefreqview.hh
	$(MOC) smtimefreqview.hh > moc_smtimefreqview.cc

moc_smspectrumwindow.cc: smspectrumwindow.hh
	$(MOC) smspectrumwindow.hh > moc_smspectrumwindow.cc

moc_smspectrumview.cc: smspectrumview.hh
	$(MOC) smspectrumview.hh > moc_smspectrumview.cc

moc_smdisplayparamwindow.cc: smdisplayparamwindow.hh
	$(MOC) smdisplayparamwindow.hh > moc_smdisplayparamwindow.cc

moc_smlpcview.cc: smlpcview.hh
	$(MOC) smlpcview.hh > moc_smlpcview.cc

moc_smlpcwindow.cc: smlpcwindow.hh
	$(MOC) smlpcwindow.hh > moc_smlpcwindow.cc

moc_smnavigatorwindow.cc: smnavigatorwindow.hh
	$(MOC) smnavigatorwindow.hh > moc_smnavigatorwindow.cc

moc_smtimefreqwinview.cc: smtimefreqwinview.hh
	$(MOC) smtimefreqwinview.hh > moc_smtimefreqwinview.cc
