#!/usr/bin/env python2

import sys
import math
from math import sin

partials = []
rate = int (sys.argv[1])
i = 2
while i < len (sys.argv):
  freq = float (sys.argv[i])
  amp = float (sys.argv[i+1])
  partials += [ (freq, amp) ]
  i += 2

phases = [ 0 ] * len (partials)
for t in range (int (rate * 0.5)):                          # generate a 0.5 second test signal
  value = 0
  for p in range (len (partials)):
    value += sin (phases[p]) * partials[p][1]
    phases[p] += partials[p][0] * 2 * math.pi / rate
  print value
