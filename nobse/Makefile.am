include $(top_srcdir)/Makefile.decl

SUBDIRS = birnet bse

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(BOOST_CPPFLAGS) $(BOOST_NUMERIC_BINDINGS_CPPFLAGS) $(SSE_FLAGS) $(FFTW_CFLAGS) $(QT_CFLAGS)

SMHDRS = nobse.hh minipcg32.hh

lib_LTLIBRARIES = libspectmorph_nobse.la
libspectmorph_nobse_la_SOURCES = nobse.cc $(SMHDRS)

spectmorph_nobse_includedir = $(includedir)/spectmorph
spectmorph_nobse_include_HEADERS = $(SMHDRS)
