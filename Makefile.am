include $(top_srcdir)/Makefile.decl

AUTOMAKE_OPTIONS = 1.9 dist-bzip2 no-dist-gzip
ACLOCAL_AMFLAGS = -I m4 -Wno-portability

SUBDIRS = lib glui vst src tests docs data

if COND_WITH_QT
  SUBDIRS += gui
endif

if COND_WITH_LV2
  SUBDIRS += lv2
endif

if COND_WITH_JACK
  SUBDIRS += jack

if COND_WITH_QT
  SUBDIRS += tools inspector
endif
endif

EXTRA_DIST += DESIGN README.md TODO Doxyfile

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = spectmorph.pc spectmorphgui.pc spectmorphglui.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
CLEANFILES += spectmorph.pc spectmorphgui.pc spectmorphglui.pc
EXTRA_DIST += spectmorph.pc.in spectmorphgui.pc.in spectmorphglui.pc.in

rebuild-meta-headers:
	make -Clib rebuild-spectmorph-hh
	make -Cglui rebuild-spectmorphglui-hh
	make -Cgui rebuild-spectmorphgui-hh
