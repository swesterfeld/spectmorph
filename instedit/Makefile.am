include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GLUI_LIBS = $(top_builddir)/glui/libspectmorphglui.la

bin_PROGRAMS = sminstedit

sminstedit_SOURCES = sminstedit.cc sminstrument.hh smwavsetbuilder.cc smwavsetbuilder.hh sminsteditwindow.hh
sminstedit_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GLUI_LIBS) $(CAIRO_LIBS)
