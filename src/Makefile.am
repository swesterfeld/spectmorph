AM_CXXFLAGS=$(BSE_CFLAGS) $(AO_CFLAGS) $(GTK_CFLAGS) -O3 -I$(top_srcdir)/lib

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

bin_PROGRAMS = smenc smplay smvisualize smextract smstrip

SMHDRS =  frame.hh noisedecoder.hh sinedecoder.hh stwutils.hh

smenc_SOURCES = smenc.cc $(SMHDRS)
smenc_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS) $(LAPACK_LIBS)

smplay_SOURCES = smplay.cc sinedecoder.cc noisedecoder.cc frame.cc $(SMHDRS)
smplay_LDADD = $(BSE_LIBS) $(AO_LIBS) $(SPECTMORPH_LIBS)

smvisualize_SOURCES = smvisualize.cc $(SMHDRS)
smvisualize_LDADD = $(BSE_LIBS) $(GTK_LIBS) $(SPECTMORPH_LIBS)

smextract_SOURCES = smextract.cc frame.cc $(SMHDRS)
smextract_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

smstrip_SOURCES = smstrip.cc $(SMHDRS)
smstrip_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)
