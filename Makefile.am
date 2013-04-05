include $(top_srcdir)/Makefile.decl

AUTOMAKE_OPTIONS = 1.9 dist-bzip2 no-dist-gzip
ACLOCAL_AMFLAGS = -I m4 -Wno-portability

if COND_WITH_BEAST
  SUBDIRS = lib gui jack src tools beast inspector python tests docs
else
  SUBDIRS = nobse lib gui jack python docs
endif

EXTRA_DIST += TODO

# === Release auxillary rule configuration ===
RELEASE_DESTHOST  = space.twc.de
RELEASE_DESTPATH  = public_html/spectmorph
RELEASE_CHANGELOG = ChangeLog
# Makefile.decl introduced rules: ChangeLog release-upload

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = spectmorph.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
CLEANFILES += spectmorph.pc
EXTRA_DIST += spectmorph.pc.in
