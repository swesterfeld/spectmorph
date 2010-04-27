AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(BOOST_CPPFLAGS) $(BOOST_NUMERIC_BINDINGS_CPPFLAGS)

SMHDRS = smafile.hh smaudio.hh smencoder.hh

lib_LTLIBRARIES = libspectmorph.la
libspectmorph_la_SOURCES = smafile.cc smencoder.cc $(SMHDRS)
libspectmorph_la_LIBADD = $(LAPACK_LIBS)
