include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(GLIB_CFLAGS) $(FFTW_CFLAGS) $(SNDFILE_CFLAGS) -I$(top_srcdir)/3rdparty/minizip \
	       -I$(top_srcdir)/3rdparty

SMHDRS = smaudio.hh smencoder.hh smnoisedecoder.hh smsinedecoder.hh \
         smmath.hh smwavset.hh smoutfile.hh sminfile.hh \
         smstdioin.hh smstdiosubin.hh smmmapin.hh smgenericin.hh \
         smgenericout.hh smstdioout.hh smmemout.hh smlivedecoder.hh \
         smrandom.hh smfft.hh smmain.hh smdebug.hh smnoisebandpartition.hh \
         smifftsynth.hh smlivedecodersource.hh smpolyphaseinter.hh \
         smjobqueue.hh smmicroconf.hh smhexstring.hh \
         smmorphplanvoice.hh smmorphplan.hh smmorphoperator.hh \
         smindex.hh smmorphsource.hh smmorphoutput.hh smmorphlinear.hh \
         smmorphoperatormodule.hh smmorphsourcemodule.hh smmorphlinearmodule.hh \
         smmorphoutputmodule.hh smwavsetrepo.hh smleakdebugger.hh \
         smmorphlfo.hh smmorphlfomodule.hh smmorphplansynth.hh \
         smmorphgrid.hh smmorphgridmodule.hh smmorphutils.hh smutils.hh \
         smminiresampler.hh smmidisynth.hh smwavdata.hh smblockutils.hh \
         smalignedarray.hh smpcg32rng.hh smproperty.hh \
         smeffectdecoder.hh smadsrenvelope.hh smsignal.hh smconfig.hh \
	 smmorphwavsource.hh smmorphwavsourcemodule.hh \
	 smwavsetbuilder.hh sminstrument.hh sminsteditsynth.hh \
	 sminstencoder.hh smbinbuffer.hh sminstenccache.hh smaudiotool.hh \
	 smzip.hh smproject.hh smsynthinterface.hh smbuilderthread.hh \
	 smuserinstrumentindex.hh smladdervcf.hh smflexadsr.hh \
	 smmodulationlist.hh smlinearsmooth.hh smpandaresampler.hh \
	 smmatharm.hh smskfilter.hh smnotifybuffer.hh smlivedecoderfilter.hh \
	 smtimeinfo.hh smdcblocker.hh smrtmemory.hh smmorphkeytrack.hh \
	 smmorphkeytrackmodule.hh smcurve.hh smmorphenvelope.hh smmorphenvelopemodule.hh \
	 smformantcorrection.hh smpitchdetect.hh

lib_LTLIBRARIES = libspectmorph.la
libspectmorph_la_SOURCES = smaudio.cc smencoder.cc smnoisedecoder.cc smsinedecoder.cc \
                           smwavset.cc smoutfile.cc sminfile.cc \
                           smstdioin.cc smstdiosubin.cc smmmapin.cc smgenericin.cc \
                           smgenericout.cc smstdioout.cc smmemout.cc smlivedecoder.cc \
                           smrandom.cc smfft.cc smmain.cc smdebug.cc smnoisebandpartition.cc \
                           smifftsynth.cc smlivedecodersource.cc smpolyphaseinter.cc \
                           smmicroconf.cc smhexstring.cc \
                           smmorphplanvoice.cc smmorphplan.cc smmorphoperator.cc \
                           smindex.cc smmorphsource.cc smmorphoutput.cc smmorphlinear.cc \
                           smmorphoperatormodule.cc smmorphsourcemodule.cc smmorphlinearmodule.cc \
                           smmorphoutputmodule.cc smwavsetrepo.cc smleakdebugger.cc \
                           smmorphlfo.cc smmorphlfomodule.cc smmorphplansynth.cc $(SMHDRS) \
                           smmorphgrid.cc \
                           smmorphgridmodule.cc smmath.cc smmorphutils.cc smutils.cc \
                           smminiresampler.cc smmidisynth.cc smwavdata.cc smblockutils.cc \
                           smalignedarray.cc smeffectdecoder.cc smadsrenvelope.cc smconfig.cc \
			   smmorphwavsource.cc smmorphwavsourcemodule.cc \
			   smwavsetbuilder.cc sminsteditsynth.cc sminstencoder.cc \
			   sminstenccache.cc smaudiotool.cc sminstrument.cc smzip.cc smproject.cc \
			   smbuilderthread.cc smproperty.cc smmodulationlist.cc smpandaresampler.cc \
			   smlivedecoderfilter.cc smtimeinfo.cc smrtmemory.cc smuserinstrumentindex.cc \
			   smmorphkeytrack.cc smmorphkeytrackmodule.cc smcurve.cc smmorphenvelope.cc \
			   smmorphenvelopemodule.cc smformantcorrection.cc smpitchdetect.cc

libspectmorph_la_LIBADD = $(LTLIBICONV) $(LAPACK_LIBS) $(FFTW_LIBS) $(GLIB_LIBS) $(SNDFILE_LIBS) $(top_builddir)/3rdparty/minizip/libminizip.la
libspectmorph_la_LDFLAGS = -no-undefined
libspectmorph_la_LIBTOOLFLAGS = --tag CXX

if COND_LINUX
  libspectmorph_la_SOURCES += smxdgdir.hh smxdgdir.cc
endif

if COND_WINDOWS
  libspectmorph_la_LIBADD += -luuid
else
  libspectmorph_la_SOURCES += smjobqueue.cc
endif

if COND_MACOS
  libspectmorph_la_SOURCES += smmacutils.mm
  libspectmorph_la_OBJCXXFLAGS = $(AM_OBJCFLAGS) $(AM_CXXFLAGS) -std=gnu++11
  libspectmorph_la_LDFLAGS += -framework Foundation
endif

spectmorphincludedir = $(includedir)/spectmorph/smcore
spectmorphinclude_HEADERS = $(SMHDRS) spectmorph.hh

EXTRA_DIST += smpolyphasecoeffs.cc

# pugixml as header only xml parsing library
EXTRA_DIST += smpugiconfig.hh smpugixml.cc smpugixml.hh

install-exec-hook: # libtool doesn't always set rpath, requiring ldconfig
	test ! -x /sbin/ldconfig -o ! -w /etc/ld.so.cache || /sbin/ldconfig $(DESTDIR)$(libdir)

rebuild-spectmorph-hh:
	echo "// SpectMorph meta-include (generated by cd lib; make rebuild-spectmorph-hh)" > gen-spectmorph.hh
	for h in $(SMHDRS); do \
	  echo "#include \"$$h\""; \
	done | sort >> gen-spectmorph.hh
	mv gen-spectmorph.hh spectmorph.hh
