include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

SMHDRS = smmorphplan.hh smmorphoperator.hh smindex.hh smmorphsource.hh smmorphplanview.hh \
         smmorphoperatorview.hh smmorphsourceview.hh smmainwindow.hh smmorphoutput.hh smmorphoutputview.hh \
         smmorphlinear.hh smmorphlinearview.hh smcomboboxoperator.hh

SMSRCS = spectmorphoscgui.cc smmorphplan.cc smmorphoperator.cc smindex.cc smmorphsource.cc smmorphplanview.cc \
         smmorphoperatorview.cc smmorphsourceview.cc smmainwindow.cc smmorphoutput.cc smmorphoutputview.cc \
         smmorphlinear.cc smmorphlinearview.cc smcomboboxoperator.cc

bin_PROGRAMS = spectmorphoscgui

spectmorphoscgui_SOURCES = $(SMSRCS) $(SMHDRS)
spectmorphoscgui_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS)
spectmorphoscgui_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)
