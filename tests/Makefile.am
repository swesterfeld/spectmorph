SMENC = $(top_builddir)/src/smenc
SMPLAY = $(top_builddir)/src/smplay
WAV2ASCII = $(top_builddir)/tools/wav2ascii
ASCII2WAV = $(top_builddir)/tools/ascii2wav
DELTA = $(top_srcdir)/tools/delta.py
SINSIGNAL = $(top_srcdir)/python/sinsignal.py
SMENCFLAGS = -O

EXTRA_DIST = saw440.wav sin440.wav sin440.py

check: saw440-test sin440-test sin440-4567-test

saw440-test:
	$(SMENC) $(SMENCFLAGS) $(srcdir)/saw440.wav saw440.sm
	$(SMPLAY) saw440.sm --no-noise --export saw440-dec.wav
	$(WAV2ASCII) $(srcdir)/saw440.wav > saw440.txt
	$(WAV2ASCII) saw440-dec.wav > saw440-dec.txt
	$(DELTA) saw440.txt saw440-dec.txt | LANG=C awk 'FNR > 3000 && FNR < 24000 { r += $$1 * $$1; c += 1 } END { print "Residual value: ",r/c; if (r/c > 5e-8) exit(1); else exit (0);}'
	rm -f saw440.sm saw440-dec.wav saw440.txt saw440-dec.txt

sin440-test:
	$(SMENC) $(SMENCFLAGS) $(srcdir)/sin440.wav sin440.sm
	$(SMPLAY) sin440.sm --no-noise --rate=96000 --export sin440-dec.wav
	$(WAV2ASCII) $(srcdir)/sin440.wav > sin440.txt
	$(WAV2ASCII) sin440-dec.wav > sin440-dec.txt
	$(DELTA) sin440.txt sin440-dec.txt | LANG=C awk 'FNR > 2900 && FNR < 16300 { r += $$1 * $$1; c += 1 } END { print "Residual value: ",r/c; if (r/c > 5e-10) exit(1); else exit (0);}'
	rm -f sin440.sm sin440-dec.wav sin440.txt sin440-dec.txt

sin440-4567-test: sin440-4567.wav
	$(SMENC) $(SMENCFLAGS) $(srcdir)/sin440-4567.wav sin440-4567.sm
	$(SMPLAY) sin440-4567.sm --no-noise --rate=96000 --export sin440-4567-dec.wav
	$(WAV2ASCII) $(srcdir)/sin440-4567.wav > sin440-4567.txt
	$(WAV2ASCII) sin440-4567-dec.wav > sin440-4567-dec.txt
	$(DELTA) sin440-4567.txt sin440-4567-dec.txt | LANG=C awk 'FNR > 3000 && FNR < 5500 { r += $$1 * $$1; c += 1 } END { print "Residual value: ",r/c; if (r/c > 5e-10) exit(1); else exit (0);}'
	rm -f sin440-4567.sm sin440-4567-dec.wav sin440-4567.txt sin440-4567-dec.txt

sin440-4567.wav: Makefile
	../python/sinsignal.py 96000 440 0.4 4567 0.5 | $(ASCII2WAV) sin440-4567.wav 96000
