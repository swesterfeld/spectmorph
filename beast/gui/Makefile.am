include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src -I$(top_srcdir)/gui

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GUI_LIBS = $(top_builddir)/gui/libspectmorphgui.la

SMHDRS =
SMSRCS =

noinst_LTLIBRARIES = libspectmorphgui.la
libspectmorphgui_la_SOURCES = $(SMSRCS) $(SMHDRS)
libspectmorphgui_la_CXXFLAGS = $(GTKMM_CFLAGS) $(AM_CXXFLAGS)

bin_PROGRAMS = spectmorphoscgui
noinst_PROGRAMS = smmorphplot smldplot

spectmorphoscgui_SOURCES = spectmorphoscgui.cc
spectmorphoscgui_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GUI_LIBS) $(GTKMM_LIBS) libspectmorphgui.la
spectmorphoscgui_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)

smmorphplot_SOURCES = smmorphplot.cc
smmorphplot_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GUI_LIBS) $(GTKMM_LIBS)
smmorphplot_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)

smldplot_SOURCES = smldplot.cc
smldplot_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GUI_LIBS) $(GTKMM_LIBS)
smldplot_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)
