SMENC = $(top_builddir)/src/smenc
SMPLAY = $(top_builddir)/src/smplay
WAV2ASCII = $(top_builddir)/tools/wav2ascii
ASCII2WAV = $(top_builddir)/tools/ascii2wav
DELTA = $(top_srcdir)/tools/delta.py
SINSIGNAL = $(top_srcdir)/python/sinsignal.py
SMENCFLAGS = -O
SMRESVALUE = $(srcdir)/smresvalue.sh

EXTRA_DIST = saw440.wav sin440.wav sin440.py

check: saw440-test sin440-test sin440-4567-test

saw440-test:
	@$(SMRESVALUE) saw440.wav 3000 24000 44100

sin440-test:
	@$(SMRESVALUE) sin440.wav 2900 16300 96000

sin440-4567-test: sin440-4567.wav
	@$(SMRESVALUE) sin440-4567.wav 3000 5500 96000

sin440-4567.wav: Makefile
	../python/sinsignal.py 96000 440 0.4 4567 0.5 | $(ASCII2WAV) sin440-4567.wav 96000
