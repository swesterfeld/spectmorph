include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

EXTRA_DIST += delta.py

noinst_PROGRAMS = ascii2wav wav2ascii imiscutter smsfimport tld

ascii2wav_SOURCES = ascii2wav.cc
ascii2wav_LDADD = $(BSE_LIBS)

wav2ascii_SOURCES = wav2ascii.cc
wav2ascii_LDADD = $(BSE_LIBS)

imiscutter_SOURCES = imiscutter.cc
imiscutter_LDADD   = $(BSE_LIBS)

smsfimport_SOURCES = smsfimport.cc
smsfimport_LDADD   = $(BSE_LIBS) $(SPECTMORPH_LIBS)

tld_SOURCES = tld.cc
tld_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)


