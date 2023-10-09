#!/usr/bin/env python3

import sys
import math
import numpy as np
import scipy
from math import log10

filename1 = sys.argv[1]
filename2 = sys.argv[2]

note = float (sys.argv[3])
freq = 440*2**((note-69)/12)

def read_samples (filename):
  f = open (filename, "r")
  result = []
  for line in f.readlines():
    if line[0] != "#":      # skip comments
      result += [ float (line) ]
  f.close()
  return result

samples1 = read_samples (filename1)
samples2 = read_samples (filename2)

assert (len (samples1) == len (samples2))

count = len (samples1)

wsize = int (44100/freq*8)
wstep = int (44100/freq)
window = np.blackman (wsize)

n = 0
signal_power = 0
delta_power = 0
while n * wstep + wsize < count:
  s = n * wstep
  array1 = np.array (samples1[s:s + wsize]) * window
  array2 = np.array (samples2[s:s + wsize]) * window

  array1 = np.pad (array1, (0, wsize), mode = 'constant')
  array2 = np.pad (array2, (0, wsize), mode = 'constant')
  array1f = scipy.fft.rfft (array1)
  array2f = scipy.fft.rfft (array2)

  for i in range (wsize):
    signal_power += abs (array1f[i]) ** 2
    delta_power += (abs (array1f[i]) - abs(array2f[i])) ** 2
  n += 1

print (10 * log10 (signal_power / delta_power))
