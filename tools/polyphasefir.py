#!/usr/bin/python

# FIR design for polyphase filter

from scipy import signal
import numpy
import sys

OVERSAMPLE = 64
SR = 48000.0
LP_START = 16000.0
LP_END   = 32000.0

def find_beta (WIDTH):
  best_beta = 0
  best_max_h = 1
  for beta in numpy.arange (0, 10, 0.1):
    b = signal.firwin (WIDTH * 2 * OVERSAMPLE + 1, 1. / OVERSAMPLE, window=('kaiser', beta))
    w, h = signal.freqz (b, worN=2**14)

    max_h = 0
    for p in range (len (h)):
      freq = (24000.0 * p) / len (h)

      if freq > 32000. / OVERSAMPLE:
        max_h = max (max_h, abs (h[p]))
    print >> sys.stderr, beta, max_h
    if (max_h < best_max_h):
      best_beta = beta
      best_max_h = max_h
  return best_beta

def design (WIDTH):
  beta = find_beta (WIDTH)
  return signal.firwin (WIDTH * 2 * OVERSAMPLE + 1, 1. / OVERSAMPLE, window=('kaiser', beta))
  #return signal.remez (WIDTH * 2 * OVERSAMPLE + 1, [0, (LP_START / SR) / OVERSAMPLE, (LP_END / SR) / OVERSAMPLE, 0.5], [1, 0])

def dump_coefficients():
 for WIDTH in range (1, 10):
   print "const double c_%d[%d] = {" % (WIDTH, WIDTH * 2 * OVERSAMPLE + 1)
   for d in design (WIDTH):
     print "  %.17g," % (d * OVERSAMPLE)
   print "};"

def dump_plot (WIDTH):
  w, h = signal.freqz (design (WIDTH), worN=2**14)

  for p in range (len (h)):
    print (24000.0 * p) / len (h), abs (h[p])

dump_plot (int (sys.argv[1]))
# dump_coefficients()
