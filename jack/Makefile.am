include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) $(JACK_CFLAGS) $(QT_CFLAGS) $(CAIRO_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/glui

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la
SPECTMORPH_GLUI_LIBS = $(top_builddir)/glui/libspectmorphglui.la

bin_PROGRAMS = smjack
noinst_PROGRAMS = smjackvel

smjack_SOURCES = smjack.cc smjack.hh
smjack_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(SPECTMORPH_GLUI_LIBS) $(QT_LIBS)

smjackvel_SOURCES = smjackvel.cc
smjackvel_LDADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(QT_LIBS)

lib_LTLIBRARIES = libspectmorphjack.la

SMJSRCS = smsimplejackplayer.cc
SMJHDRS = smsimplejackplayer.hh

libspectmorphjack_la_SOURCES = $(SMJSRCS) $(SMJHDRS)
libspectmorphjack_la_CXXFLAGS = $(AM_CXXFLAGS)
libspectmorphjack_la_LIBADD = $(BSE_LIBS) $(JACK_LIBS) $(SPECTMORPH_LIBS) $(QTCORE_LIBS)

install-exec-hook: # libtool doesn't always set rpath, requiring ldconfig
	test ! -x /sbin/ldconfig -o ! -w /etc/ld.so.cache || /sbin/ldconfig $(DESTDIR)$(libdir)
