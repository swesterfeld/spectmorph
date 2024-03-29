include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(QT_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la $(QTCORE_LIBS)
SPECTMORPH_GUI_LIBS = $(top_builddir)/gui/libspectmorphgui.la
SPECTMORPH_JACK_LIBS = $(top_builddir)/jack/libspectmorphjack.la

EXTRA_DIST += delta.py smeval.py smutils.py polyphasefir.py

bin_PROGRAMS = smsfimport smsampleedit smenccache
bin_SCRIPTS  = sminstbuilder

noinst_PROGRAMS = ascii2wav wav2ascii imiscutter tld smfiledump smrunplan \
		  smfileedit smevalplayer smfcompare

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

smfileedit_SOURCES = smfileedit.cc
smfileedit_LDADD   = $(BSE_LIBS) $(SPECTMORPH_LIBS)

tld_SOURCES = tld.cc
tld_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

if COND_WITH_AO
smlive_SOURCES = smlive.cc
smlive_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS) $(AO_LIBS)
smlive_CXXFLAGS = $(AM_CXXFLAGS) $(AO_CFLAGS)
noinst_PROGRAMS += smlive
endif

smenccache_SOURCES = smenccache.cc
smenccache_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

smsampleedit_SOURCES = smsampleedit.cc smsampleedit.hh
nodist_smsampleedit_SOURCES = moc_smsampleedit.cc
smsampleedit_LDADD   = $(BSE_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GUI_LIBS) $(SPECTMORPH_JACK_LIBS) $(QT_LIBS)
smsampleedit_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/gui -I$(top_srcdir)/jack

CLEANFILES += $(nodist_smsampleedit_SOURCES)

smrunplan_SOURCES = smrunplan.cc
smrunplan_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)
smrunplan_CXXFLAGS = $(AM_CXXFLAGS)

smevalplayer_SOURCES = smevalplayer.cc
smevalplayer_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_JACK_LIBS)
smevalplayer_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/jack

smfcompare_SOURCES = smfcompare.cc
smfcompare_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)
smfcompare_CXXFLAGS = $(AM_CXXFLAGS)


install-exec-hook:
	@chmod +x $(DESTDIR)$(bindir)/sminstbuilder

moc_smsampleedit.cc: smsampleedit.hh
	$(MOC) $(srcdir)/smsampleedit.hh > moc_smsampleedit.cc
