include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(AO_CFLAGS) -I$(top_srcdir)/lib

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

bin_PROGRAMS = smenc smplay smtool smstrip

if !COND_WINDOWS
# smwavset needs job queue
bin_PROGRAMS += smwavset
endif

SMHDRS = stwutils.hh

smenc_SOURCES = smenc.cc $(SMHDRS)
smenc_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

smplay_SOURCES = smplay.cc $(SMHDRS)
smplay_LDADD = $(BSE_LIBS) $(AO_LIBS) $(SPECTMORPH_LIBS)

smtool_SOURCES = smtool.cc $(SMHDRS)
smtool_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

smstrip_SOURCES = smstrip.cc $(SMHDRS)
smstrip_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

smwavset_SOURCES = smwavset.cc $(SMHDRS)
smwavset_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)
