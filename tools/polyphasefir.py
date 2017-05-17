#!/usr/bin/python

# FIR design for polyphase filter

from scipy import signal

OVERSAMPLE = 64
SR = 48000.0
LP_START = 16000.0
LP_END   = 32000.0

def design (WIDTH):
  #return signal.firwin (WIDTH * 2 * OVERSAMPLE + 1, 1. / OVERSAMPLE, window=('kaiser', 7.5)) <- 7
  #return signal.firwin (WIDTH * 2 * OVERSAMPLE + 1, 1. / OVERSAMPLE, window=('kaiser', 5.0)) <- 5
  return signal.firwin (WIDTH * 2 * OVERSAMPLE + 1, 1. / OVERSAMPLE, width = 0.5 / OVERSAMPLE)
  #return signal.remez (WIDTH * 2 * OVERSAMPLE + 1, [0, (LP_START / SR) / OVERSAMPLE, (LP_END / SR) / OVERSAMPLE, 0.5], [1, 0])

w, h = signal.freqz (design (8), worN=2**14)

for p in range (len (h)):
  print (24000.0 * p) / len (h), abs (h[p])

# for WIDTH in range (1, 10):
#   print "const double c_%d[%d] = {" % (WIDTH, WIDTH * 2 * OVERSAMPLE + 1)
#   for d in design (WIDTH):
#     print "  %.17g," % (d * OVERSAMPLE)
#   print "};"
