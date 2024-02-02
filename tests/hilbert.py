#!/usr/bin/env python3

import sys
import math
from scipy.signal import hilbert
import numpy as np

def read_samples():
  result = []
  for line in sys.stdin.readlines():
    if line[0] != "#":      # skip comments
      result.append (float (line))
  return result

signal = read_samples()

analytic_signal = hilbert (signal)
amplitude_envelope = np.abs (analytic_signal)
instantaneous_phase = np.unwrap (np.angle (analytic_signal))
instantaneous_frequency = (np.diff (instantaneous_phase) / (2.0 * np.pi) * 48000)

for i in range (analytic_signal.shape[0] - 1):
  print (analytic_signal[i].real, analytic_signal[i].imag, amplitude_envelope[i], instantaneous_frequency[i])
