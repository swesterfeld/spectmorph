AM_CXXFLAGS=$(BSE_CFLAGS) $(AO_CFLAGS) -O3

bin_PROGRAMS = smenc smplay

SMHDRS =  frame.hh stwafile.hh noisedecoder.hh sinedecoder.hh stwutils.hh stwaudio.idl

CLEANFILES = stwaudio.hh

smenc_SOURCES = smenc.cc stwafile.cc $(SMHDRS)
smenc_LDADD = $(BSE_LIBS)

smplay_SOURCES = smplay.cc sinedecoder.cc noisedecoder.cc stwafile.cc frame.cc $(SMHDRS)
smplay_LDADD = $(BSE_LIBS) $(AO_LIBS)

$(srcdir)/smenc.cc: stwaudio.hh
$(srcdir)/smplay.cc: stwaudio.hh

stwaudio.hh: $(srcdir)/stwaudio.idl
	( echo "#ifndef STWAUDIO_HH" ; \
	echo "#define STWAUDIO_HH" ; \
	sfidl --plugin $(srcdir)/stwaudio.idl ; \
	echo "#endif" ) > $@
