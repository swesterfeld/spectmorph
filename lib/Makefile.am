include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -O3 $(BOOST_CPPFLAGS) $(BOOST_NUMERIC_BINDINGS_CPPFLAGS) $(SSE_FLAGS) $(FFTW_CFLAGS) $(QT_CFLAGS)

SMHDRS = smaudio.hh smencoder.hh smnoisedecoder.hh smsinedecoder.hh \
         smmath.hh smwavset.hh smoutfile.hh sminfile.hh \
         smstdioin.hh smstdiosubin.hh smmmapin.hh smgenericin.hh \
         smgenericout.hh smstdioout.hh smmemout.hh smlivedecoder.hh \
         smrandom.hh smfft.hh smmain.hh smdebug.hh smnoisebandpartition.hh \
         smifftsynth.hh smlivedecodersource.hh smpolyphaseinter.hh \
         smjobqueue.hh smwavloader.hh smmicroconf.hh smhexstring.hh \
         smmorphplanvoice.hh smmorphplan.hh smmorphoperator.hh \
         smindex.hh smmorphsource.hh smmorphoutput.hh smmorphlinear.hh \
         smmorphoperatormodule.hh smmorphsourcemodule.hh smmorphlinearmodule.hh \
         smmorphoutputmodule.hh smwavsetrepo.hh smleakdebugger.hh smobject.hh \
         smlpc.hh smmorphlfo.hh smmorphlfomodule.hh smmorphplansynth.hh \
         smmorphgrid.hh

lib_LTLIBRARIES = libspectmorph.la
libspectmorph_la_SOURCES = smaudio.cc smencoder.cc smnoisedecoder.cc smsinedecoder.cc \
                           smwavset.cc smoutfile.cc sminfile.cc \
                           smstdioin.cc smstdiosubin.cc smmmapin.cc smgenericin.cc \
                           smgenericout.cc smstdioout.cc smmemout.cc smlivedecoder.cc \
                           smrandom.cc smfft.cc smmain.cc smdebug.cc smnoisebandpartition.cc \
                           smifftsynth.cc smlivedecodersource.cc smpolyphaseinter.cc \
                           smjobqueue.cc smwavloader.cc smmicroconf.cc smhexstring.cc \
                           smmorphplanvoice.cc smmorphplan.cc smmorphoperator.cc \
                           smindex.cc smmorphsource.cc smmorphoutput.cc smmorphlinear.cc \
                           smmorphoperatormodule.cc smmorphsourcemodule.cc smmorphlinearmodule.cc \
                           smmorphoutputmodule.cc smwavsetrepo.cc smleakdebugger.cc smobject.cc \
                           smlpc.cc smmorphlfo.cc smmorphlfomodule.cc smmorphplansynth.cc $(SMHDRS) \
                           moc_smmorphplan.cc moc_smmorphlinear.cc moc_smmorphoperator.cc \
                           moc_smmorphoutput.cc smmorphgrid.cc moc_smmorphgrid.cc \
                           smmorphgridmodule.cc

libspectmorph_la_LIBADD = $(LAPACK_LIBS) $(FFTW_LIBS) $(QT_LIBS) $(BSE_LIBS)

spectmorphincludedir = $(includedir)/spectmorph
spectmorphinclude_HEADERS = $(SMHDRS)

moc_smmorphplan.cc: smmorphplan.hh
	moc smmorphplan.hh > moc_smmorphplan.cc

moc_smmorphlinear.cc: smmorphlinear.hh
	moc smmorphlinear.hh > moc_smmorphlinear.cc

moc_smmorphoperator.cc: smmorphoperator.hh
	moc smmorphoperator.hh > moc_smmorphoperator.cc

moc_smmorphoutput.cc: smmorphoutput.hh
	moc smmorphoutput.hh > moc_smmorphoutput.cc

moc_smmorphgrid.cc: smmorphgrid.hh
	moc smmorphgrid.hh > moc_smmorphgrid.cc
