AM_CXXFLAGS=$(BSE_CFLAGS) $(AO_CFLAGS) -O3

bin_PROGRAMS = smenc smplay

smenc_SOURCES = smenc.cc stwafile.cc
smenc_LDADD = $(BSE_LIBS)

smplay_SOURCES = smplay.cc sinedecoder.cc noisedecoder.cc stwafile.cc frame.cc
smplay_LDADD = $(BSE_LIBS) $(AO_LIBS)

stwaudio.hh: stwaudio.idl
	( echo "#ifndef STWAUDIO_HH" ; \
	echo "#define STWAUDIO_HH" ; \
	sfidl --plugin stwaudio.idl ; \
	echo "#endif" ) > stwaudio.hh
