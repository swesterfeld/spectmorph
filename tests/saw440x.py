#!/usr/bin/env python3

from math import cos, sin, pi

rate = 44100.0
freq = 440.0
phase = 0.0

def cwindow (x):
  return (cos (x * pi) + 1) / 2

sig = []

for i in range (22050):
  j = 1
  value = 0
  while j * freq < 20000:
    value += cwindow (j * freq / (0.5 * rate)) * sin (phase * j) / j
    j += 1
  sig.append (value)
  phase += freq * 2.0 * pi / rate
  while phase > 2.0 * pi:
    phase -= 2.0 *pi

# normalize
minmax = 0
for value in sig:
  minmax = max (abs (value), minmax)

for value in sig:
  print ("%.17g" % (value / minmax))
