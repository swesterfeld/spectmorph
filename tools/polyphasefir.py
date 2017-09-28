#!/usr/bin/env python2

# FIR design for polyphase filter

from scipy import signal
from numpy.fft import rfft
import numpy
import math
import sys

OVERSAMPLE = 64
SR = 48000.0
LP_START = 16000.0
LP_END   = 32000.0

def find_beta (WIDTH):
  best_beta = 0
  best_max_h = 1
  for beta in numpy.arange (0, 10, 0.1):
    b = signal.firwin (WIDTH * 2 * OVERSAMPLE - 1, 1. / OVERSAMPLE, window=('kaiser', beta))
    w, h = signal.freqz (b, worN=2**14)

    max_h = 0
    for p in range (len (h)):
      freq = (OVERSAMPLE * 24000.0 * p) / len (h)

      if freq > 32000.:
        max_h = max (max_h, abs (h[p]))

    if (max_h < best_max_h):
      best_beta = beta
      best_max_h = max_h
  return best_beta

def design (WIDTH):
  beta = find_beta (WIDTH)
  return signal.firwin (WIDTH * 2 * OVERSAMPLE - 1, 1. / OVERSAMPLE, window=('kaiser', beta))
  #return signal.remez (WIDTH * 2 * OVERSAMPLE - 1, [0, (LP_START / SR) / OVERSAMPLE, (LP_END / SR) / OVERSAMPLE, 0.5], [1, 0])

def dump_coefficients():
 for WIDTH in range (1, 10):
   print "static const double c_%d[%d] = {" % (WIDTH, WIDTH * 2 * OVERSAMPLE - 1)
   for d in design (WIDTH):
     print "  %.17g," % (d * OVERSAMPLE)
   print "};"

def dump_plot (WIDTH):
  w, h = signal.freqz (design (WIDTH), worN=2**14)

  for p in range (len (h)):
    print (OVERSAMPLE * 24000.0 * p) / len (h), abs (h[p])

def test_filter (WIDTH, FREQ):
  # use filter for upsampling a sine signal, compute fft
  x = []
  for i in range (int (SR * 0.1)):
    x.append (math.sin (i * FREQ / SR * 2 * math.pi))
    for j in range (OVERSAMPLE - 1):
      x.append (0)

  window = numpy.kaiser (65536, 12.0)

  fft_in = []
  y = signal.convolve (x, design (WIDTH))
  for i in range (10 * OVERSAMPLE, len (x) - 10 * OVERSAMPLE):
    #print "%d %.4f %.4f" % (i, x[i], y[i + WIDTH * OVERSAMPLE])
    if len (fft_in) < len (window):
      fft_in.append (y[i + WIDTH * OVERSAMPLE] * window[len (fft_in)])

  fft_out = rfft (fft_in)

  for i in range (len (fft_out)):
    print (i * SR * OVERSAMPLE) / len (fft_in), abs (fft_out[i])

dump_plot (int (sys.argv[1]))
# dump_coefficients()
# test_filter (int (sys.argv[1]), float (sys.argv[2]))
