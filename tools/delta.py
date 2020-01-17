#!/usr/bin/env python3

import sys

def read_samples (filename):
  samples = []
  f = open (filename)
  for line in f.readlines():
    if line[0] != "#":      # skip comments
      samples += [ float (line) ]
  return samples

a_samples = read_samples (sys.argv[1])
b_samples = read_samples (sys.argv[2])

for x in range (0, max (len (a_samples), len (b_samples))):
  diff = 0
  if x < len (a_samples):
    diff += a_samples[x]
  if x < len (b_samples):
    diff -= b_samples[x]
  print (diff)
