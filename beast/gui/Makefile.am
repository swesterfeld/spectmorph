include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

SMHDRS = smmorphplanview.hh \
         smmorphoperatorview.hh smmorphsourceview.hh smmainwindow.hh smmorphoutputview.hh \
         smmorphlinearview.hh smcomboboxoperator.hh smrenameoperatordialog.hh \
         smmoveindicator.hh

SMSRCS = smmorphplanview.cc \
         smmorphoperatorview.cc smmorphsourceview.cc smmainwindow.cc smmorphoutputview.cc \
         smmorphlinearview.cc smcomboboxoperator.cc smrenameoperatordialog.cc \
         smmoveindicator.cc

noinst_LTLIBRARIES = libspectmorphgui.la
libspectmorphgui_la_SOURCES = $(SMSRCS) $(SMHDRS)
libspectmorphgui_la_CXXFLAGS = $(GTKMM_CFLAGS) $(AM_CXXFLAGS)

bin_PROGRAMS = spectmorphoscgui smrunplan

spectmorphoscgui_SOURCES = spectmorphoscgui.cc
spectmorphoscgui_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS) libspectmorphgui.la
spectmorphoscgui_CXXFLAGS = $(GTKMM_CFLAGS)  $(AM_CXXFLAGS)

smrunplan_SOURCES = smrunplan.cc
smrunplan_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS) libspectmorphgui.la
smrunplan_CXXFLAGS = $(GTKMM_CFLAGS) $(AM_CXXFLAGS)
