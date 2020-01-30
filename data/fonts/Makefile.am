include $(top_srcdir)/Makefile.decl

FONTS_ARCHIVE = fonts.tar.xz
EXTRA_DIST += $(FONTS_ARCHIVE)

install-data-hook:
	tar xf $(FONTS_ARCHIVE) -C $(DESTDIR)$(pkgdatadir) --no-same-owner --no-same-permissions

# uninstall all files that the tarball contains
#  - ignore directories
uninstall-local:
	tar --list -f $(FONTS_ARCHIVE) | grep -v '/$$' | tr '\n' '\0' | (cd $(DESTDIR)$(pkgdatadir) ; xargs -0 rm -f)
