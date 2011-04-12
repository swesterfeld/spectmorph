include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GUI_LIBS = $(top_builddir)/gui/libspectmorphgui.la

EXTRA_DIST += delta.py

bin_PROGRAMS = smsfimport

noinst_PROGRAMS = ascii2wav wav2ascii imiscutter tld smfiledump smsampleedit

ascii2wav_SOURCES = ascii2wav.cc
ascii2wav_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

wav2ascii_SOURCES = wav2ascii.cc
wav2ascii_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

imiscutter_SOURCES = imiscutter.cc
imiscutter_LDADD   = $(BSE_LIBS) $(SPECTMORPH_LIBS)

smsfimport_SOURCES = smsfimport.cc
smsfimport_LDADD   = $(BSE_LIBS) $(SPECTMORPH_LIBS)

smfiledump_SOURCES = smfiledump.cc
smfiledump_LDADD   = $(BSE_LIBS) $(SPECTMORPH_LIBS)

tld_SOURCES = tld.cc
tld_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

smsampleedit_SOURCES = smsampleedit.cc
smsampleedit_LDADD   = $(BSE_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GUI_LIBS) $(GTKMM_LIBS)
smsampleedit_CXXFLAGS = $(GTKMM_CFLAGS) $(AM_CXXFLAGS) -I$(top_srcdir)/gui
