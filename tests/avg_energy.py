#!/usr/bin/env python2

import sys
import math

start_line = int (sys.argv[1])
end_line = int (sys.argv[2])

def read_samples():
  result = []
  line_number = 1
  for line in sys.stdin.readlines():
    if line[0] != "#":      # skip comments
      if (line_number >= start_line) and (line_number <= end_line):
        result += [ float (line) ]
      line_number += 1
  return result

signal = read_samples()

energy = 0
for x in signal:
  energy += x * x

energy /= len (signal)
print 10 * math.log10 (energy)
