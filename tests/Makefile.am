include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS=$(BSE_CFLAGS) -I$(top_srcdir)/lib -O3 $(SSE_FLAGS) $(SIGCXX_CFLAGS)

ASCII2WAV = $(top_builddir)/tools/ascii2wav
SINSIGNAL = $(top_srcdir)/python/sinsignal.py
SMRESVALUE = $(top_builddir)/tests/smresvalue.sh

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

EXTRA_DIST += saw440.wav sin440.wav sin440.py
CLEANFILES += sin440-4567.wav

TESTS = testfastsin testblob testfft testisincos testnoisemodes testifftsynth testppinter testgenid

noinst_PROGRAMS = $(TESTS) testrandom testfftperf testnoise testrandperf testaafilter testnoiseperf \
        testjobqueue testrefptr testparamupdate testloopindex testoutfileperf testlpc

testfastsin_SOURCES = testfastsin.cc
testfastsin_LDADD = $(BSE_LIBS)

testblob_SOURCES = testblob.cc
testblob_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testgenid_SOURCES = testgenid.cc
testgenid_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testfft_SOURCES = testfft.cc
testfft_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testfftperf_SOURCES = testfftperf.cc
testfftperf_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testparamupdate_SOURCES = testparamupdate.cc
testparamupdate_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testrandom_SOURCES = testrandom.cc
testrandom_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testnoise_SOURCES = testnoise.cc
testnoise_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testnoisemodes_SOURCES = testnoisemodes.cc
testnoisemodes_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testrandperf_SOURCES = testrandperf.cc
testrandperf_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testnoiseperf_SOURCES = testnoiseperf.cc
testnoiseperf_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testisincos_SOURCES = testisincos.cc
testisincos_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testifftsynth_SOURCES = testifftsynth.cc
testifftsynth_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testaafilter_SOURCES = testaafilter.cc
testaafilter_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testppinter_SOURCES = testppinter.cc
testppinter_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testjobqueue_SOURCES = testjobqueue.cc
testjobqueue_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testrefptr_SOURCES = testrefptr.cc
testrefptr_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testloopindex_SOURCES = testloopindex.cc
testloopindex_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testoutfileperf_SOURCES = testoutfileperf.cc
testoutfileperf_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

testlpc_SOURCES = testlpc.cc
testlpc_LDADD = $(BSE_LIBS) $(SPECTMORPH_LIBS)

check: saw440-test sin440-test sin440-4567-test

saw440-test:
	@$(SMRESVALUE) saw440.wav 3000 24000 44100

sin440-test:
	@$(SMRESVALUE) sin440.wav 2900 16300 96000

sin440-4567-test: sin440-4567.wav
	@$(SMRESVALUE) sin440-4567.wav 3000 5500 96000

sin440-4567.wav: Makefile
	$(SINSIGNAL) 96000 440 0.4 4567 0.5 | $(ASCII2WAV) sin440-4567.wav 96000
