include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(QT_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src -I$(top_srcdir)/gui

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GUI_LIBS = $(top_builddir)/gui/libspectmorphgui.la

bin_PROGRAMS = spectmorphoscgui

spectmorphoscgui_SOURCES = spectmorphoscgui.cc moc_spectmorphoscgui.cc spectmorphoscgui.hh
spectmorphoscgui_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GUI_LIBS) $(GTKMM_LIBS) $(QT_LIBS)
spectmorphoscgui_CXXFLAGS = $(AM_CXXFLAGS)

moc_spectmorphoscgui.cc: spectmorphoscgui.hh
	moc spectmorphoscgui.hh > moc_spectmorphoscgui.cc
