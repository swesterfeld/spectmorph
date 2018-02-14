include $(top_srcdir)/Makefile.decl

SUBDIRS = pugl

if COND_LINUX
  PUGL_LIBS = pugl/libspectmorphpugl.la -lX11 -lGL
endif

if COND_MACOS
  PUGL_LIBS = pugl/libspectmorphpugl.la
  PUGL_LDFLAGS = -framework Cocoa -framework OpenGL
endif

if COND_WINDOWS
  PUGL_LIBS = pugl/libspectmorphpugl.la -lopengl32 -lgdi32 -luser32
endif

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la $(QTCORE_LIBS)

noinst_LTLIBRARIES = libspectmorphglui.la

SMSRCS = smwidget.cc smwindow.cc smmorphplanview.cc smmorphplanwindow.cc \
	 smmorphsourceview.cc smmorphoutputview.cc smcomboboxoperator.cc \
	 smmorphlinearview.cc smpropertyview.cc smmorphlfoview.cc

SMHDRS = smlabel.hh smslider.hh smwidget.hh smwindow.hh smframe.hh \
	 smcombobox.hh smdrawutils.hh smscrollbar.hh smmenubar.hh \
	 smmorphplanwindow.hh smmorphplancontrol.hh smmorphplanview.hh \
	 smfixedgrid.hh smmorphoperatorview.hh smmorphsourceview.hh \
	 smmorphoutputview.hh smcomboboxoperator.hh smmorphlinearview.hh \
	 smcheckbox.hh smpropertyview.hh smmorphlfoview.hh

libspectmorphglui_la_SOURCES = $(SMSRCS) $(SMHDRS)
libspectmorphglui_la_CXXFLAGS = $(AM_CXXFLAGS)

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

STATIC_DLL_LIBS = \
  .libs/libspectmorphglui.a \
  pugl/.libs/libspectmorphpugl.a \
  $(top_builddir)/lib/.libs/libspectmorph.a

dll:
	$(CXX) -shared -o smvstplugin.dll smvstplugin.cc -static $(AM_CFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $(STATIC_DLL_LIBS) $(QTCORE_LIBS) $(BSE_LIBS) $(FFTW_LIBS) $(CAIRO_LIBS) -lopengl32 -lgdi32 -luser32 -liconv
