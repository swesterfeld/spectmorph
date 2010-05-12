AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(BOOST_CPPFLAGS) $(BOOST_NUMERIC_BINDINGS_CPPFLAGS) $(SSE_FLAGS)

SMHDRS = smafile.hh smaudio.hh smencoder.hh smnoisedecoder.hh smsinedecoder.hh smframe.hh smmath.hh

lib_LTLIBRARIES = libspectmorph.la
libspectmorph_la_SOURCES = smafile.cc smencoder.cc smnoisedecoder.cc smsinedecoder.cc smframe.cc $(SMHDRS)
libspectmorph_la_LIBADD = $(LAPACK_LIBS)
