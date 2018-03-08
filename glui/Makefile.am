include $(top_srcdir)/Makefile.decl

SUBDIRS = pugl

if COND_LINUX
  PUGL_LIBS = pugl/libspectmorphpugl.la -lX11 -lGL
  SMFDIALOG = smextfiledialog.cc
endif

if COND_MACOS
  PUGL_LIBS = pugl/libspectmorphpugl.la
  PUGL_LDFLAGS = -framework Cocoa -framework OpenGL
  SMFDIALOG = smmacfiledialog.mm

  # FIXME: there may be a cleaner solution to compile in C++11 mode
  libspectmorphglui_la_OBJCXXFLAGS = $(AM_OBJCFLAGS) $(AM_CXXFLAGS) -std=gnu++11
endif

if COND_WINDOWS
  PUGL_LIBS = pugl/libspectmorphpugl.la -lopengl32 -lgdi32 -luser32 -lcomdlg32
  SMFDIALOG = smwinfiledialog.cc
endif

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la $(QTCORE_LIBS)

lib_LTLIBRARIES = libspectmorphglui.la

SMSRCS = smwidget.cc smwindow.cc smmorphplanview.cc smmorphplanwindow.cc \
	 smmorphsourceview.cc smmorphoutputview.cc smcomboboxoperator.cc \
	 smmorphlinearview.cc smpropertyview.cc smmorphlfoview.cc \
	 smmorphgridview.cc smmorphgridwidget.cc \
	 smmorphoperatorview.cc $(SMFDIALOG) smrenameopdialog.cc \
	 smdialog.cc smaboutdialog.cc

SMHDRS = smlabel.hh smslider.hh smwidget.hh smwindow.hh smframe.hh \
	 smcombobox.hh smdrawutils.hh smscrollbar.hh smmenubar.hh \
	 smmorphplanwindow.hh smmorphplancontrol.hh smmorphplanview.hh \
	 smfixedgrid.hh smmorphoperatorview.hh smmorphsourceview.hh \
	 smmorphoutputview.hh smcomboboxoperator.hh smmorphlinearview.hh \
	 smcheckbox.hh smpropertyview.hh smmorphlfoview.hh smscrollview.hh \
	 smbutton.hh smtoolbutton.hh smmorphgridview.hh smmorphgridwidget.hh \
	 smlineedit.hh smnativefiledialog.hh \
	 smrenameopdialog.hh smdialog.hh smaboutdialog.hh

libspectmorphglui_la_SOURCES = $(SMSRCS) $(SMHDRS)
libspectmorphglui_la_CXXFLAGS = $(AM_CXXFLAGS)
libspectmorphglui_la_LIBTOOLFLAGS = --tag CXX

AM_CXXFLAGS += $(QTCORE_CFLAGS) $(BSE_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src -I$(top_srcdir)/glui -DPUGL_HAVE_GL -DPUGL_HAVE_CAIRO -I$(top_srcdir)/vst

smuitest_SOURCES = smuitest.cc
smuitest_LDADD = libspectmorphglui.la $(SPECTMORPH_LIBS) $(BSE_LIBS) $(CAIRO_LIBS) $(PUGL_LIBS)
smuitest_LDFLAGS = $(PUGL_LDFLAGS)

smscrolltest_SOURCES = smscrolltest.cc
smscrolltest_LDADD = libspectmorphglui.la $(SPECTMORPH_LIBS) $(BSE_LIBS) $(CAIRO_LIBS) $(PUGL_LIBS)
smscrolltest_LDFLAGS = $(PUGL_LDFLAGS)

smplantest_SOURCES = smplantest.cc
smplantest_LDADD = libspectmorphglui.la $(SPECTMORPH_LIBS) $(BSE_LIBS) $(CAIRO_LIBS) $(PUGL_LIBS)
smplantest_LDFLAGS = $(PUGL_LDFLAGS)

noinst_PROGRAMS = smuitest smplantest smscrolltest

plugindir = $(abs_builddir)/VST_TEST_DIR
plugin_LTLIBRARIES = smvstplugin.la

SMVSTHEADERS = smvstplugin.hh smvstui.hh smvstcommon.hh
SMVSTSOURCES = smvstplugin.cc smvstui.cc

smvstplugin_la_SOURCES = $(SMVSTSOURCES) $(SMVSTHEADERS)
smvstplugin_la_LIBADD  = libspectmorphglui.la $(SPECTMORPH_LIBS) $(BSE_LIBS) $(CAIRO_LIBS) $(PUGL_LIBS)
smvstplugin_la_LDFLAGS = -avoid-version -module -export-symbols-regex "VSTPluginMain|main"

if COND_WINDOWS
STATIC_DLL_LIBS = \
  .libs/libspectmorphglui.a \
  pugl/.libs/libspectmorphpugl.a \
  $(top_builddir)/lib/.libs/libspectmorph.a

install-exec-hook: smvstplugin.dll
	cp smvstplugin.dll $(plugindir)

smvstplugin.dll: $(plugin_LTLIBRARIES)
	$(CXX) -shared -o smvstplugin.dll $(srcdir)/smvstplugin.cc $(srcdir)/smvstui.cc -static $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_DLL_LIBS) $(QTCORE_LIBS) $(BSE_LIBS) $(FFTW_LIBS) $(CAIRO_LIBS) -lopengl32 -lgdi32 -luser32 -lcomdlg32 -liconv
endif
