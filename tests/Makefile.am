include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(BSE_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src $(QTCORE_CFLAGS)

ASCII2WAV = $(top_builddir)/tools/ascii2wav
SINSIGNAL = $(top_srcdir)/python/sinsignal.py
SMRESVALUE = $(top_srcdir)/tests/smresvalue.sh

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la $(QTCORE_LIBS)

EXTRA_DIST += saw440.wav sin440.wav sin440.py saw440x.py avg_energy.py sn_delta.py whitenoise.py \
        smresvalue.sh tune-test.sh test-norm.sh
CLEANFILES += sin440-4567.wav saw440x.wav

TESTS = testfastsin testblob testfft testisincos testnoisemodes testifftsynth testppinter testgenid \
        testidb testifreq testbesseli0

noinst_PROGRAMS = $(TESTS) testrandom testfftperf testnoise testrandperf testaafilter testnoiseperf \
        testrefptr testparamupdate testloopindex testoutfileperf testlpc testrootfinder \
        testlsfperf testmutexperf testsortfreqs testconvperf testminires testnoisesr testlsfplot \
        testlpca testblockperf testlowpass1 testxparam testmidisynth testadsr testadsrdecay testsignal

if !COND_WINDOWS
noinst_PROGRAMS += testjobqueue
endif

testfastsin_SOURCES = testfastsin.cc
testfastsin_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testblob_SOURCES = testblob.cc
testblob_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testgenid_SOURCES = testgenid.cc
testgenid_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testfft_SOURCES = testfft.cc
testfft_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testfftperf_SOURCES = testfftperf.cc
testfftperf_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testparamupdate_SOURCES = testparamupdate.cc
testparamupdate_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testrandom_SOURCES = testrandom.cc
testrandom_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testxparam_SOURCES = testxparam.cc
testxparam_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testnoise_SOURCES = testnoise.cc
testnoise_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testnoisemodes_SOURCES = testnoisemodes.cc
testnoisemodes_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testrandperf_SOURCES = testrandperf.cc
testrandperf_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testnoiseperf_SOURCES = testnoiseperf.cc
testnoiseperf_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testblockperf_SOURCES = testblockperf.cc
testblockperf_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testisincos_SOURCES = testisincos.cc
testisincos_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testifftsynth_SOURCES = testifftsynth.cc
testifftsynth_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testaafilter_SOURCES = testaafilter.cc
testaafilter_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testppinter_SOURCES = testppinter.cc
testppinter_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testjobqueue_SOURCES = testjobqueue.cc
testjobqueue_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testrefptr_SOURCES = testrefptr.cc
testrefptr_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testloopindex_SOURCES = testloopindex.cc
testloopindex_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testoutfileperf_SOURCES = testoutfileperf.cc
testoutfileperf_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testlpc_SOURCES = testlpc.cc
testlpc_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testrootfinder_SOURCES = testrootfinder.cc
testrootfinder_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testlsfperf_SOURCES = testlsfperf.cc
testlsfperf_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testlsfplot_SOURCES = testlsfplot.cc
testlsfplot_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testmutexperf_SOURCES = testmutexperf.cc
testmutexperf_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testsortfreqs_SOURCES = testsortfreqs.cc
testsortfreqs_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testidb_SOURCES = testidb.cc
testidb_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testifreq_SOURCES = testifreq.cc
testifreq_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testconvperf_SOURCES = testconvperf.cc
testconvperf_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testminires_SOURCES = testminires.cc
testminires_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testnoisesr_SOURCES = testnoisesr.cc
testnoisesr_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testlpca_SOURCES = testlpca.cc
testlpca_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testlowpass1_SOURCES = testlowpass1.cc
testlowpass1_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testbesseli0_SOURCES = testbesseli0.cc
testbesseli0_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testadsr_SOURCES = testadsr.cc
testadsr_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testadsrdecay_SOURCES = testadsrdecay.cc
testadsrdecay_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testsignal_SOURCES = testsignal.cc
testsignal_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

testmidisynth_SOURCES = testmidisynth.cc
testmidisynth_LDADD = $(SPECTMORPH_LIBS) $(BSE_LIBS)

check: saw440-test saw440x-test sin440-test sin440-4567-test TXT-saw440-test TXT-sin440-test TXT-sin440-4567-test \
       TXT-sin100-test TXT-sin140-test tune-test test-norm

saw440-test:
	@$(SMRESVALUE) saw440.wav 3000 24000 44100 440

saw440x-test: saw440x.wav
	@$(SMRESVALUE) saw440x.wav 2000 20000 44100 440

sin440-test:
	@$(SMRESVALUE) sin440.wav 3500 16300 96000 440

sin440-4567-test: sin440-4567.wav
	@$(SMRESVALUE) sin440-4567.wav 3500 6000 96000 440

TXT-saw440-test:
	$(top_srcdir)/tests/saw440x.py > TXT-saw440.txt
	@$(SMRESVALUE) TXT-saw440.txt 2000 20000 44100 440

TXT-sin440-test:
	$(SINSIGNAL) 44100 440 0.9 > TXT-sin440.txt
	@$(SMRESVALUE) TXT-sin440.txt 2000 20000 44100 440

TXT-sin100-test:
	$(SINSIGNAL) 44100 100 0.9 > TXT-sin100.txt
	@$(SMRESVALUE) TXT-sin100.txt 2000 20000 44100 100

TXT-sin140-test:
	$(SINSIGNAL) 44100 140 0.9 > TXT-sin140.txt
	@$(SMRESVALUE) TXT-sin140.txt 2000 20000 44100 140

TXT-sin440-4567-test:
	$(SINSIGNAL) 44100 440 0.4 4567 0.5 > TXT-sin440-4567.txt
	@$(SMRESVALUE) TXT-sin440-4567.txt 2000 20000 44100 440

sin440-4567.wav: Makefile
	$(SINSIGNAL) 96000 440 0.4 4567 0.5 | $(ASCII2WAV) sin440-4567.wav 96000

saw440x.wav: saw440x.py
	$(top_srcdir)/tests/saw440x.py | $(ASCII2WAV) saw440x.wav 44100

tune-test:
	$(top_srcdir)/tests/tune-test.sh

test-norm:
	$(top_srcdir)/tests/test-norm.sh
