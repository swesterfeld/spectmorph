include $(top_srcdir)/Makefile.decl

AUTOMAKE_OPTIONS = 1.9 dist-bzip2 no-dist-gzip
ACLOCAL_AMFLAGS = -I m4 -Wno-portability

SUBDIRS = 3rdparty lib glui vst clap src tools docs data

if COND_WITH_LV2
  SUBDIRS += lv2
endif

if COND_WITH_JACK
  SUBDIRS += jack instedit

if COND_WITH_QT
  SUBDIRS += inspector
endif
endif

SUBDIRS += tests

EXTRA_DIST += DESIGN README.md TODO Doxyfile

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = spectmorph.pc spectmorphglui.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
CLEANFILES += spectmorph.pc spectmorphglui.pc
EXTRA_DIST += spectmorph.pc.in spectmorphglui.pc.in

rebuild-meta-headers:
	make -Clib rebuild-spectmorph-hh
	make -Cglui rebuild-spectmorphglui-hh

tidy:
	clang-tidy-17 $(shell git ls-tree --name-only HEAD -r | grep -v ^3rdparty | grep -e '.cc') -- -DPUGL_HAVE_GL -DPUGL_HAVE_CAIRO $(GLIB_CFLAGS) -std=c++17 -I. -I3rdparty/minizip -I3rdparty -Ilib -Iglui -Isrc -Igui -Ijack $(CAIRO_CFLAGS) $(QT_CFLAGS)
