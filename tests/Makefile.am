SMENC = $(top_builddir)/src/smenc
SMPLAY = $(top_builddir)/src/smplay
WAV2ASCII = $(top_builddir)/tools/wav2ascii
DELTA = $(top_srcdir)/tools/delta.py

EXTRA_DIST = saw440.wav

check: saw440-test

saw440-test:
	$(SMENC) -O $(srcdir)/saw440.wav saw440.sm
	$(SMPLAY) saw440.sm --export saw440-dec.wav
	$(WAV2ASCII) $(srcdir)/saw440.wav > saw440.txt
	$(WAV2ASCII) saw440-dec.wav > saw440-dec.txt
	$(DELTA) saw440.txt saw440-dec.txt | LANG=C awk 'FNR > 3000 && FNR < 24000 { r += $$1 * $$1; c += 1 } END { print "Residual value: ",r/c; if (r/c > 5e-8) exit(1); else exit (0);}'
	rm -f saw440.sm saw440-dec.wav saw440.txt saw440-dec.txt
