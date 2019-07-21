include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GLUI_LIBS = $(top_builddir)/glui/libspectmorphglui.la

# don't install for now, as accessing user defined instrument is difficult
# use JACK/VST/LV2 instead
noinst_PROGRAMS = sminstedit

sminstedit_SOURCES = sminstedit.cc
sminstedit_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GLUI_LIBS) $(CAIRO_LIBS)
