include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

EXTRA_DIST += delta.py

bin_PROGRAMS = smsfimport

noinst_PROGRAMS = ascii2wav wav2ascii imiscutter tld sminspector

ascii2wav_SOURCES = ascii2wav.cc
ascii2wav_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

wav2ascii_SOURCES = wav2ascii.cc
wav2ascii_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

imiscutter_SOURCES = imiscutter.cc
imiscutter_LDADD   = $(BSE_LIBS) $(SPECTMORPH_LIBS)

smsfimport_SOURCES = smsfimport.cc
smsfimport_LDADD   = $(BSE_LIBS) $(SPECTMORPH_LIBS)

sminspector_SOURCES = sminspector.cc
sminspector_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS)
sminspector_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)

tld_SOURCES = tld.cc
tld_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)


