AM_CXXFLAGS=$(BSE_CFLAGS) $(AO_CFLAGS) $(GTK_CFLAGS) -O3

bin_PROGRAMS = smenc smplay smvisualize smextract smstrip

SMHDRS =  frame.hh stwafile.hh noisedecoder.hh sinedecoder.hh stwutils.hh smaudio.hh

CLEANFILES = stwaudio.hh

smenc_SOURCES = smenc.cc stwafile.cc $(SMHDRS)
smenc_LDADD = $(BSE_LIBS)

smplay_SOURCES = smplay.cc sinedecoder.cc noisedecoder.cc stwafile.cc frame.cc $(SMHDRS)
smplay_LDADD = $(BSE_LIBS) $(AO_LIBS)

smvisualize_SOURCES = smvisualize.cc stwafile.cc $(SMHDRS)
smvisualize_LDADD = $(BSE_LIBS) $(GTK_LIBS)

smextract_SOURCES = smextract.cc stwafile.cc frame.cc $(SMHDRS)
smextract_LDADD = $(BSE_LIBS)

smstrip_SOURCES = smstrip.cc stwafile.cc $(SMHDRS)
smstrip_LDADD = $(BSE_LIBS)
