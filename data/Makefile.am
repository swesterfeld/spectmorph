include $(top_srcdir)/Makefile.decl

SUBDIRS = templates

# install smjack .desktop file
dotdesktopdir = $(datarootdir)/applications
dotdesktop_DATA = smjack.desktop

EXTRA_DIST += $(dotdesktop_DATA)

# install related pixmaps
sharedpixmapsdir = $(datarootdir)/pixmaps
sharedpixmaps_DATA = smjack.png

EXTRA_DIST += $(sharedpixmaps_DATA)

# instruments: distribute / install if available, ignore otherwise
INSTRUMENTS_ARCHIVE = $(srcdir)/spectmorph-instruments-$(VERSION).tar.xz

dist-hook:
	if test -f $(INSTRUMENTS_ARCHIVE); then \
	  cp $(INSTRUMENTS_ARCHIVE) $(distdir); \
	fi

install-data-hook:
	if test -f $(INSTRUMENTS_ARCHIVE); then \
	  tar xf $(INSTRUMENTS_ARCHIVE) -C $(DESTDIR)$(pkgdatadir); \
	fi

# uninstall all files that the tarball contains
#  - ignore directories
uninstall-local:
	if test -f $(INSTRUMENTS_ARCHIVE); then \
	  tar --list -f $(INSTRUMENTS_ARCHIVE) | grep -v '/$$' | tr '\n' '\0' | (cd $(DESTDIR)$(pkgdatadir) ; xargs -0 rm -f); \
	fi
