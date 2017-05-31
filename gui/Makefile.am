include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(QT_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

SMHDRS = smzoomcontroller.hh smcomboboxoperator.hh smmorphplanwindow.hh smrenameoperatordialog.hh \
         smmorphplanview.hh smmorphsourceview.hh smmorphoperatorview.hh smmoveindicator.hh \
         smmorphlinearview.hh smmorphoutputview.hh smsampleview.hh smlpcztrans.hh smmorphlfoview.hh \
         smmorphgridview.hh smmorphgridwidget.hh smled.hh smmorphplancontrol.hh smpropertyview.hh

SMSRCS = smzoomcontroller.cc smcomboboxoperator.cc smmorphplanwindow.cc smrenameoperatordialog.cc \
         smmorphplanview.cc smmorphsourceview.cc smmorphoperatorview.cc smmoveindicator.cc \
         smmorphlinearview.cc smmorphoutputview.cc smsampleview.cc smlpcztrans.cc smmorphlfoview.cc \
         moc_smmorphplanwindow.cc moc_smmorphsourceview.cc moc_smmorphoutputview.cc moc_smcomboboxoperator.cc \
         moc_smmorphoperatorview.cc moc_smrenameoperatordialog.cc moc_smmorphplanview.cc moc_smmorphlinearview.cc \
         moc_smmorphlfoview.cc moc_smsampleview.cc moc_smzoomcontroller.cc smmorphgridview.cc moc_smmorphgridview.cc \
         smmorphgridwidget.cc moc_smmorphgridwidget.cc smled.cc moc_smled.cc smmorphplancontrol.cc moc_smmorphplancontrol.cc \
         smpropertyview.cc moc_smpropertyview.cc

lib_LTLIBRARIES = libspectmorphgui.la

libspectmorphgui_la_SOURCES = $(SMSRCS) $(SMHDRS)
libspectmorphgui_la_CXXFLAGS = $(AM_CXXFLAGS)
libspectmorphgui_la_LIBADD = $(BSE_LIBS) $(SPECTMORPH_LIBS) $(QT_LIBS)

smguiincludedir = $(includedir)/spectmorph/smgui
smguiinclude_HEADERS = $(SMHDRS)

moc_smmorphplanwindow.cc: smmorphplanwindow.hh
	$(MOC) smmorphplanwindow.hh > moc_smmorphplanwindow.cc

moc_smmorphsourceview.cc: smmorphsourceview.hh
	$(MOC) smmorphsourceview.hh > moc_smmorphsourceview.cc

moc_smmorphoutputview.cc: smmorphoutputview.hh
	$(MOC) smmorphoutputview.hh > moc_smmorphoutputview.cc

moc_smcomboboxoperator.cc: smcomboboxoperator.hh
	$(MOC) smcomboboxoperator.hh > moc_smcomboboxoperator.cc

moc_smmorphoperatorview.cc: smmorphoperatorview.hh
	$(MOC) smmorphoperatorview.hh > moc_smmorphoperatorview.cc

moc_smrenameoperatordialog.cc: smrenameoperatordialog.hh
	$(MOC) smrenameoperatordialog.hh > moc_smrenameoperatordialog.cc

moc_smmorphplanview.cc: smmorphplanview.hh
	$(MOC) smmorphplanview.hh > moc_smmorphplanview.cc

moc_smmorphlinearview.cc: smmorphlinearview.hh
	$(MOC) smmorphlinearview.hh > moc_smmorphlinearview.cc

moc_smmorphlfoview.cc: smmorphlfoview.hh
	$(MOC) smmorphlfoview.hh > moc_smmorphlfoview.cc

moc_smsampleview.cc: smsampleview.hh
	$(MOC) smsampleview.hh > moc_smsampleview.cc

moc_smzoomcontroller.cc: smzoomcontroller.hh
	$(MOC) smzoomcontroller.hh > moc_smzoomcontroller.cc

moc_smmorphgridview.cc: smmorphgridview.hh
	$(MOC) smmorphgridview.hh > moc_smmorphgridview.cc

moc_smmorphgridwidget.cc: smmorphgridwidget.hh
	$(MOC) smmorphgridwidget.hh > moc_smmorphgridwidget.cc

moc_smled.cc: smled.hh
	$(MOC) smled.hh > moc_smled.cc

moc_smmorphplancontrol.cc: smmorphplancontrol.hh
	$(MOC) smmorphplancontrol.hh > moc_smmorphplancontrol.cc

moc_smpropertyview.cc: smpropertyview.hh
	$(MOC) smpropertyview.hh > moc_smpropertyview.cc
