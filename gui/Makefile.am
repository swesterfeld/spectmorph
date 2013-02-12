include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(SSE_FLAGS) $(QT_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src -D_BIRNET_SOURCE_EXTENSIONS

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

SMHDRS = smzoomcontroller.hh smcomboboxoperator.hh smmorphplanwindow.hh smrenameoperatordialog.hh \
         smmorphplanview.hh smmorphsourceview.hh smmorphoperatorview.hh smmoveindicator.hh \
         smmorphlinearview.hh smmorphoutputview.hh smsampleview.hh smlpcztrans.hh smmorphlfoview.hh

SMSRCS = smzoomcontroller.cc smcomboboxoperator.cc smmorphplanwindow.cc smrenameoperatordialog.cc \
         smmorphplanview.cc smmorphsourceview.cc smmorphoperatorview.cc smmoveindicator.cc \
         smmorphlinearview.cc smmorphoutputview.cc smsampleview.cc smlpcztrans.cc smmorphlfoview.cc \
         moc_smmorphplanwindow.cc moc_smmorphsourceview.cc moc_smmorphoutputview.cc moc_smcomboboxoperator.cc \
         moc_smmorphoperatorview.cc moc_smrenameoperatordialog.cc moc_smmorphplanview.cc moc_smmorphlinearview.cc \
         moc_smmorphlfoview.cc moc_smsampleview.cc moc_smzoomcontroller.cc smmorphgridview.cc moc_smmorphgridview.cc \
         smmorphgridwidget.cc moc_smmorphgridwidget.cc

lib_LTLIBRARIES = libspectmorphgui.la

libspectmorphgui_la_SOURCES = $(SMSRCS) $(SMHDRS)
libspectmorphgui_la_CXXFLAGS = $(GTKMM_CFLAGS) $(AM_CXXFLAGS)
libspectmorphgui_la_LIBADD = $(BSE_LIBS) $(SPECTMORPH_LIBS) $(GTKMM_LIBS)

moc_smmorphplanwindow.cc: smmorphplanwindow.hh
	moc smmorphplanwindow.hh > moc_smmorphplanwindow.cc

moc_smmorphsourceview.cc: smmorphsourceview.hh
	moc smmorphsourceview.hh > moc_smmorphsourceview.cc

moc_smmorphoutputview.cc: smmorphoutputview.hh
	moc smmorphoutputview.hh > moc_smmorphoutputview.cc

moc_smcomboboxoperator.cc: smcomboboxoperator.hh
	moc smcomboboxoperator.hh > moc_smcomboboxoperator.cc

moc_smmorphoperatorview.cc: smmorphoperatorview.hh
	moc smmorphoperatorview.hh > moc_smmorphoperatorview.cc

moc_smrenameoperatordialog.cc: smrenameoperatordialog.hh
	moc smrenameoperatordialog.hh > moc_smrenameoperatordialog.cc

moc_smmorphplanview.cc: smmorphplanview.hh
	moc smmorphplanview.hh > moc_smmorphplanview.cc

moc_smmorphlinearview.cc: smmorphlinearview.hh
	moc smmorphlinearview.hh > moc_smmorphlinearview.cc

moc_smmorphlfoview.cc: smmorphlfoview.hh
	moc smmorphlfoview.hh > moc_smmorphlfoview.cc

moc_smsampleview.cc: smsampleview.hh
	moc smsampleview.hh > moc_smsampleview.cc

moc_smzoomcontroller.cc: smzoomcontroller.hh
	moc smzoomcontroller.hh > moc_smzoomcontroller.cc

moc_smmorphgridview.cc: smmorphgridview.hh
	moc smmorphgridview.hh > moc_smmorphgridview.cc

moc_smmorphgridwidget.cc: smmorphgridwidget.hh
	moc smmorphgridwidget.hh > moc_smmorphgridwidget.cc
