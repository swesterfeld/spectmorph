include $(top_srcdir)/Makefile.decl

AM_CXXFLAGS += $(GLIB_CFLAGS) $(SNDFILE_CFLAGS) -I$(top_srcdir)/lib -I$(top_srcdir)/src

ASCII2WAV = $(top_builddir)/tools/ascii2wav
SINSIGNAL = $(srcdir)/sinsignal.py
SMRESVALUE = $(srcdir)/smresvalue.sh

SPECTMORPH_LIBS = $(top_builddir)/lib/libspectmorph.la

EXTRA_DIST += saw440.wav sin440.wav sin440.py saw440x.py avg_energy.py sn_delta.py whitenoise.py \
        sinsignal.py smresvalue.sh tune-test.sh test-norm.sh test-porta.sh hilbert.py \
	post-install-test.sh
CLEANFILES += sin440-4567.wav saw440x.wav

TESTS_ENVIRONMENT = SPECTMORPH_MAKE_CHECK=1

TESTS = testfastsin testblob testisincos testnoisemodes testifftsynth testppinter testgenid \
        testidb testifreq testbesseli0 testsse testblockmath testceventlock testpitchdetect

noinst_PROGRAMS = $(TESTS) testrandom testfftperf testnoise testrandperf testaafilter testnoiseperf \
        testparamupdate testloopindex testoutfileperf \
        testsortfreqs testconvperf testminires testnoisesr \
        testblockperf testlowpass1 testxparam testmidisynth testadsr testadsrdecay testsignal \
	teststrformat testvelocity testinstbuild testautovol testwavdata testzip testuindexperf \
	testlfo testsmdirs testladdervcf testpandaperf testnotifyperf testpropperf testroundperf \
	testpsola testcurve

REFS = ref/1-instrument.ref ref/2-instruments-linear-gui.ref ref/2-instruments-linear-lfo.ref \
       ref/2-instruments-unison.ref ref/2x2-instruments-grid-gui.ref ref/aurora.ref ref/cheese-cake-bass.ref \
       ref/empty.ref ref/keytrack-choir-ah-oh.ref ref/liquid-silver.ref ref/mars.ref ref/saturn.ref ref/stringbrass-fusion.ref

EXTRA_DIST += $(REFS)

if !COND_WINDOWS
noinst_PROGRAMS += testjobqueue
endif

testfastsin_SOURCES = testfastsin.cc
testfastsin_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testblob_SOURCES = testblob.cc
testblob_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testgenid_SOURCES = testgenid.cc
testgenid_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testfftperf_SOURCES = testfftperf.cc
testfftperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testparamupdate_SOURCES = testparamupdate.cc
testparamupdate_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testrandom_SOURCES = testrandom.cc
testrandom_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testxparam_SOURCES = testxparam.cc
testxparam_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testnoise_SOURCES = testnoise.cc
testnoise_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testnoisemodes_SOURCES = testnoisemodes.cc
testnoisemodes_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testrandperf_SOURCES = testrandperf.cc
testrandperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testnoiseperf_SOURCES = testnoiseperf.cc
testnoiseperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testblockperf_SOURCES = testblockperf.cc
testblockperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testisincos_SOURCES = testisincos.cc
testisincos_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testifftsynth_SOURCES = testifftsynth.cc
testifftsynth_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testaafilter_SOURCES = testaafilter.cc
testaafilter_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testppinter_SOURCES = testppinter.cc
testppinter_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testjobqueue_SOURCES = testjobqueue.cc
testjobqueue_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testloopindex_SOURCES = testloopindex.cc
testloopindex_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testoutfileperf_SOURCES = testoutfileperf.cc
testoutfileperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testsortfreqs_SOURCES = testsortfreqs.cc
testsortfreqs_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testidb_SOURCES = testidb.cc
testidb_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testifreq_SOURCES = testifreq.cc
testifreq_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testconvperf_SOURCES = testconvperf.cc
testconvperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testminires_SOURCES = testminires.cc
testminires_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testnoisesr_SOURCES = testnoisesr.cc
testnoisesr_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testlowpass1_SOURCES = testlowpass1.cc
testlowpass1_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testbesseli0_SOURCES = testbesseli0.cc
testbesseli0_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testadsr_SOURCES = testadsr.cc
testadsr_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testadsrdecay_SOURCES = testadsrdecay.cc
testadsrdecay_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testsignal_SOURCES = testsignal.cc
testsignal_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testmidisynth_SOURCES = testmidisynth.cc
testmidisynth_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

teststrformat_SOURCES = teststrformat.cc
teststrformat_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testvelocity_SOURCES = testvelocity.cc
testvelocity_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testinstbuild_SOURCES = testinstbuild.cc
testinstbuild_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testautovol_SOURCES = testautovol.cc
testautovol_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testwavdata_SOURCES = testwavdata.cc
testwavdata_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testzip_SOURCES = testzip.cc
testzip_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testuindexperf_SOURCES = testuindexperf.cc
testuindexperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testlfo_SOURCES = testlfo.cc
testlfo_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testsmdirs_SOURCES = testsmdirs.cc
testsmdirs_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testladdervcf_SOURCES = testladdervcf.cc
testladdervcf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testsse_SOURCES = testsse.cc
testsse_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testpandaperf_SOURCES = testpandaperf.cc
testpandaperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testnotifyperf_SOURCES = testnotifyperf.cc
testnotifyperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testpropperf_SOURCES = testpropperf.cc
testpropperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testroundperf_SOURCES = testroundperf.cc
testroundperf_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testpsola_SOURCES = testpsola.cc
testpsola_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testcurve_SOURCES = testcurve.cc
testcurve_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testblockmath_SOURCES = testblockmath.cc
testblockmath_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testceventlock_SOURCES = testceventlock.cc
testceventlock_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

testpitchdetect_SOURCES = testpitchdetect.cc
testpitchdetect_LDADD = $(SPECTMORPH_LIBS) $(GLIB_LIBS)

check: saw440-test saw440x-test sin440-test sin440-4567-test TXT-saw440-test TXT-sin440-test TXT-sin440-4567-test \
       TXT-sin100-test TXT-sin140-test tune-test test-norm test-porta

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
	$(SINSIGNAL) 96000 440 0.4 4567 0.5 | $(ASCII2WAV) sin440-4567.wav -r 96000

saw440x.wav: saw440x.py
	$(top_srcdir)/tests/saw440x.py | $(ASCII2WAV) saw440x.wav -r 44100

tune-test:
	$(top_srcdir)/tests/tune-test.sh

test-norm:
	$(top_srcdir)/tests/test-norm.sh

test-porta:
	$(top_srcdir)/tests/test-porta.sh

list-refs:
	@echo $(REFS)
