AM_CXXFLAGS=$(BSE_CFLAGS) -I$(top_srcdir)/lib -O3 -mmmx -msse -ftree-vectorize

ASCII2WAV = $(top_builddir)/tools/ascii2wav
SINSIGNAL = $(top_srcdir)/python/sinsignal.py
SMRESVALUE = $(top_builddir)/tests/smresvalue.sh

EXTRA_DIST = saw440.wav sin440.wav sin440.py
CLEANFILES = sin440-4567.wav

TESTS = testfastsin

noinst_PROGRAMS = $(TESTS)

testfastsin_SOURCES = testfastsin.cc
testfastsin_LDADD = $(BSE_LIBS)

check: saw440-test sin440-test sin440-4567-test

saw440-test:
	@$(SMRESVALUE) saw440.wav 3000 24000 44100

sin440-test:
	@$(SMRESVALUE) sin440.wav 2900 16300 96000

sin440-4567-test: sin440-4567.wav
	@$(SMRESVALUE) sin440-4567.wav 3000 5500 96000

sin440-4567.wav: Makefile
	$(SINSIGNAL) 96000 440 0.4 4567 0.5 | $(ASCII2WAV) sin440-4567.wav 96000
