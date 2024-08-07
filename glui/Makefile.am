include $(top_srcdir)/Makefile.decl

SUBDIRS = pugl

if COND_LINUX
  PUGL_LIBS = pugl/libspectmorphpugl.la -lX11 $(GL_LIBS)
  SMFDIALOG = smlinuxfiledialog.cc
endif

if COND_MACOS
  PUGL_LIBS = pugl/libspectmorphpugl.la
  PUGL_LDFLAGS = -framework Cocoa -framework OpenGL
  SMFDIALOG = smmacfiledialog.mm

  # FIXME: there may be a cleaner solution to compile in C++17mode
  libspectmorphglui_la_OBJCXXFLAGS = $(AM_OBJCFLAGS) $(AM_CXXFLAGS) -std=gnu++17
endif

if COND_WINDOWS
  PUGL_LIBS = pugl/libspectmorphpugl.la -lopengl32 -lgdi32 -luser32 -lcomdlg32
  SMFDIALOG = smwinfiledialog.cc
endif

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

lib_LTLIBRARIES = libspectmorphglui.la

SMSRCS = smwidget.cc smwindow.cc smmorphplanview.cc smmorphplanwindow.cc \
	 smmorphsourceview.cc smmorphoutputview.cc smcomboboxoperator.cc \
	 smmorphlinearview.cc smpropertyview.cc smmorphlfoview.cc \
	 smmorphgridview.cc smmorphgridwidget.cc smmessagebox.cc \
	 smmorphoperatorview.cc $(SMFDIALOG) \
	 smdialog.cc smaboutdialog.cc smmorphplancontrol.cc \
	 smdrawutils.cc smrenameopwindow.cc smmorphwavsourceview.cc \
	 smtimer.cc smshortcut.cc smparamlabel.cc smeventloop.cc \
	 sminsteditwindow.cc smlineedit.cc smmorphkeytrackview.cc \
	 smmorphcurvewidget.cc smmorphenvelopeview.cc

SMHDRS = smlabel.hh smslider.hh smwidget.hh smwindow.hh smframe.hh \
	 smcombobox.hh smdrawutils.hh smscrollbar.hh smmenubar.hh \
	 smmorphplanwindow.hh smmorphplancontrol.hh smmorphplanview.hh \
	 smfixedgrid.hh smmorphoperatorview.hh smmorphsourceview.hh \
	 smmorphoutputview.hh smcomboboxoperator.hh smmorphlinearview.hh \
	 smcheckbox.hh smpropertyview.hh smmorphlfoview.hh smscrollview.hh \
	 smbutton.hh smtoolbutton.hh smmorphgridview.hh smmorphgridwidget.hh \
	 smlineedit.hh smnativefiledialog.hh smled.hh \
	 smdialog.hh smaboutdialog.hh smoperatorlayout.hh \
	 smmessagebox.hh smrenameopwindow.hh smoutputadsrwidget.hh \
	 smmorphwavsourceview.hh smsamplewidget.hh sminsteditwindow.hh \
	 smtimer.hh smshortcut.hh sminsteditparams.hh \
	 smparamlabel.hh smeventloop.hh smprogressbar.hh sminsteditnote.hh \
	 smsimplelines.hh smoperatorrolemap.hh smlistbox.hh \
	 smcontrolview.hh smpropertyviewedit.hh smcontrolstatus.hh \
	 smvoicestatus.hh smpathdialog.hh smloadstereodialog.hh \
	 smbankeditwindow.hh smcreatebankwindow.hh smclickablelabel.hh \
	 sminsteditvolume.hh smvumeter.hh smvolumeresetdialog.hh \
	 smmorphkeytrackview.hh smmorphcurvewidget.hh smmorphenvelopeview.hh

libspectmorphglui_la_SOURCES = $(SMSRCS) $(SMHDRS)
libspectmorphglui_la_CXXFLAGS = $(AM_CXXFLAGS)
libspectmorphglui_la_LIBADD = $(SPECTMORPH_LIBS) $(PUGL_LIBS) $(FREETYPE_LIBS) $(CAIRO_LIBS) $(GLIB_LIBS)
libspectmorphglui_la_LDFLAGS = -no-undefined $(PUGL_LDFLAGS)
libspectmorphglui_la_LIBTOOLFLAGS = --tag CXX

AM_CXXFLAGS += $(GLIB_CFLAGS) $(SNDFILE_CFLAGS) $(FREETYPE_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src -I$(top_srcdir)/glui -DPUGL_HAVE_GL -DPUGL_HAVE_CAIRO -I$(top_srcdir)/vst

smuitest_SOURCES = smuitest.cc
smuitest_LDADD = libspectmorphglui.la $(SPECTMORPH_LIBS) $(GLIB_LIBS) $(CAIRO_LIBS) $(PUGL_LIBS)
smuitest_LDFLAGS = $(PUGL_LDFLAGS)

smscrolltest_SOURCES = smscrolltest.cc
smscrolltest_LDADD = libspectmorphglui.la $(SPECTMORPH_LIBS) $(GLIB_LIBS) $(CAIRO_LIBS) $(PUGL_LIBS)
smscrolltest_LDFLAGS = $(PUGL_LDFLAGS)

noinst_PROGRAMS = smuitest smscrolltest

smgluiincludedir = $(includedir)/spectmorph/smglui
smgluiinclude_HEADERS = $(SMHDRS) spectmorphglui.hh

rebuild-spectmorphglui-hh:
	echo "// SpectMorph GUI meta-include (generated by cd glui; make rebuild-spectmorphglui-hh)" > gen-spectmorphglui.hh
	echo "#include \"spectmorph.hh\"" >> gen-spectmorphglui.hh
	for h in $(SMHDRS); do \
	  echo "#include \"$$h\""; \
	done | sort >> gen-spectmorphglui.hh
	mv gen-spectmorphglui.hh spectmorphglui.hh
