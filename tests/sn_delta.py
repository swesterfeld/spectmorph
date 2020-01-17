#!/usr/bin/env python3

import sys
import math

filename1  = sys.argv[1]
filename2  = sys.argv[2]
start_line = int (sys.argv[3])
end_line   = int (sys.argv[4])
bound      = float (sys.argv[5])

def read_samples (filename):
  f = open (filename, "r")
  result = []
  line_number = 1
  for line in f.readlines():
    if line[0] != "#":      # skip comments
      if (line_number >= start_line) and (line_number <= end_line):
        result += [ float (line) ]
      line_number += 1
  f.close()
  return result

samples1 = read_samples (filename1)
samples2 = read_samples (filename2)

assert (len (samples1) == len (samples2))

count = len (samples1)

s1_power = 0
s2_power = 0
delta_power = 0

for i in range (count):
  s1 = samples1[i]
  s2 = samples2[i]
  delta = s1 - s2

  s1_power += s1 * s1
  s2_power += s2 * s2
  delta_power += delta * delta

s1_power /= count
s2_power /= count
delta_power /= count
#print s1_power, s2_power, delta_power
#print 10 * math.log10 (s1_power), 10 * math.log10 (s2_power), 10 * math.log10 (delta_power),
sn_db = (10 * math.log10 (s1_power / delta_power))
if sn_db < bound:
  print ("sn_delta.py: comparing %s and %s results in S/N %.3f (but should be at least %.3f)" % (filename1, filename2, sn_db, bound), file=sys.stderr)
  sys.exit (1)
print ("%.3f" % sn_db)
