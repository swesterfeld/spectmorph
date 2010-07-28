include $(top_srcdir)/Makefile.decl

AUTOMAKE_OPTIONS = 1.9 dist-bzip2 no-dist-gzip
ACLOCAL_AMFLAGS = -I m4 -Wno-portability

SUBDIRS = lib src tools beast jack python tests docs
EXTRA_DIST += TODO

# === Release auxillary rule configuration ===
RELEASE_DESTHOST  = space.twc.de
RELEASE_DESTPATH  = public_html/spectmorph
RELEASE_CHANGELOG = ChangeLog
# Makefile.decl introduced rules: ChangeLog release-upload
