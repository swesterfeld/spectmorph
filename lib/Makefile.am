AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(BOOST_CPPFLAGS) $(BOOST_NUMERIC_BINDINGS_CPPFLAGS) $(SSE_FLAGS)

SMHDRS = smaudio.hh smencoder.hh smnoisedecoder.hh smsinedecoder.hh \
         smframe.hh smmath.hh smwavset.hh smoutfile.hh sminfile.hh \
         smstdioin.hh smstdiosubin.hh smmmapin.hh smgenericin.hh

lib_LTLIBRARIES = libspectmorph.la
libspectmorph_la_SOURCES = smaudio.cc smencoder.cc smnoisedecoder.cc smsinedecoder.cc \
                           smframe.cc smwavset.cc smoutfile.cc sminfile.cc \
                           smstdioin.cc smstdiosubin.cc smmmapin.cc smgenericin.cc $(SMHDRS)
libspectmorph_la_LIBADD = $(LAPACK_LIBS)
