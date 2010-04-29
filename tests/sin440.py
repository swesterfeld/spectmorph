#!/usr/bin/env python

from math import sin, pi

rate = 96000.0
freq = 440.0
phase = 0.0

for i in range (20000):
  print sin (phase) * 0.9
  phase += freq * 2.0 * pi / rate
  while phase > 2.0 * pi:
    phase -= 2.0 *pi
