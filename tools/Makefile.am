include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(GLIB_CFLAGS) $(JACK_CFLAGS) $(SNDFILE_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GUI_LIBS = $(top_builddir)/gui/libspectmorphgui.la
SPECTMORPH_JACK_LIBS = $(top_builddir)/jack/libspectmorphjack.la

EXTRA_DIST += delta.py smeval.py smutils.py polyphasefir.py

bin_PROGRAMS =
bin_SCRIPTS  = sminstbuilder

noinst_PROGRAMS = ascii2wav wav2ascii imiscutter tld smfiledump smrunplan \
		  smfileedit smfcompare

ascii2wav_SOURCES = ascii2wav.cc
ascii2wav_LDADD = $(GLIB_LIBS) $(SPECTMORPH_LIBS)

wav2ascii_SOURCES = wav2ascii.cc
wav2ascii_LDADD = $(GLIB_LIBS) $(SPECTMORPH_LIBS)

imiscutter_SOURCES = imiscutter.cc
imiscutter_LDADD   = $(GLIB_LIBS) $(SPECTMORPH_LIBS)

if !COND_WINDOWS
smsfimport_SOURCES = smsfimport.cc
smsfimport_LDADD   = $(GLIB_LIBS) $(SPECTMORPH_LIBS)

smenccache_SOURCES = smenccache.cc
smenccache_LDADD = $(GLIB_LIBS) $(SPECTMORPH_LIBS)

bin_PROGRAMS += smsfimport smenccache
endif

smfiledump_SOURCES = smfiledump.cc
smfiledump_LDADD   = $(GLIB_LIBS) $(SPECTMORPH_LIBS)

smfileedit_SOURCES = smfileedit.cc
smfileedit_LDADD   = $(GLIB_LIBS) $(SPECTMORPH_LIBS)

tld_SOURCES = tld.cc
tld_LDADD = $(GLIB_LIBS) $(SPECTMORPH_LIBS)

if COND_WITH_AO
smlive_SOURCES = smlive.cc
smlive_LDADD = $(GLIB_LIBS) $(SPECTMORPH_LIBS) $(AO_LIBS)
smlive_CXXFLAGS = $(AM_CXXFLAGS) $(AO_CFLAGS)
noinst_PROGRAMS += smlive
endif

smrunplan_SOURCES = smrunplan.cc
smrunplan_LDADD = $(GLIB_LIBS) $(SPECTMORPH_LIBS)
smrunplan_CXXFLAGS = $(AM_CXXFLAGS)

if COND_WITH_JACK
smevalplayer_SOURCES = smevalplayer.cc
smevalplayer_LDADD = $(GLIB_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_JACK_LIBS)
smevalplayer_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/jack
noinst_PROGRAMS += smevalplayer
endif

smfcompare_SOURCES = smfcompare.cc
smfcompare_LDADD = $(GLIB_LIBS) $(SPECTMORPH_LIBS)
smfcompare_CXXFLAGS = $(AM_CXXFLAGS)


install-exec-hook:
	@chmod +x $(DESTDIR)$(bindir)/sminstbuilder
