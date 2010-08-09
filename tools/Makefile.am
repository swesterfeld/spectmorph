include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

EXTRA_DIST += delta.py

noinst_PROGRAMS = ascii2wav wav2ascii smsetload

ascii2wav_SOURCES = ascii2wav.cc
ascii2wav_LDADD = $(BSE_LIBS)

wav2ascii_SOURCES = wav2ascii.cc
wav2ascii_LDADD = $(BSE_LIBS)

smsetload_SOURCES = smsetload.cc
smsetload_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)
