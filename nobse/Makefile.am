include $(top_srcdir)/Makefile.decl

SUBDIRS = birnet bse

AM_CXXFLAGS=$(GLIB_CFLAGS) -O3 $(SSE_FLAGS)

SMHDRS = nobse.hh minipcg32.hh rapicorn.hh

lib_LTLIBRARIES = libspectmorph_nobse.la
libspectmorph_nobse_la_SOURCES = nobse.cc $(SMHDRS)
libspectmorph_nobse_la_LIBADD = $(GLIB_LIBS)

spectmorph_nobse_includedir = $(includedir)/spectmorph
spectmorph_nobse_include_HEADERS = $(SMHDRS)
