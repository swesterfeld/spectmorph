include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS = $(BSE_CFLAGS) $(JACK_CFLAGS) $(GTKMM_CFLAGS) -I$(top_srcdir)/lib

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

bin_PROGRAMS = smjack smjackvel

smjack_SOURCES = smjack.cc
smjack_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS)

smjackvel_SOURCES = smjackvel.cc
smjackvel_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS)
