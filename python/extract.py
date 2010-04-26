#!/usr/bin/env python

import sys
import spectmorph
import math
from math import sin, cos, pi

try:
  audio = spectmorph.load_stwafile (sys.argv[1])
except:
  print "cannot load file:", sys.argv[1]

print "#Audio File:"
print "#fundamental_freq = %f Hz" % audio.fundamental_freq
print "#mix_freq = %f Hz" % audio.mix_freq
print "#frame_size_ms = %f ms" % audio.frame_size_ms
print "#frame_step_ms = %f ms" % audio.frame_step_ms
print "#zeropad = %d" % audio.zeropad
print "#len (audio.contents) = %d" % len (audio.contents)
print "#audio.contents[10].freqs = %s" % audio.contents[10].freqs
print "#len (audio.contents[10].freqs) = %d" % len (audio.contents[10].freqs)
print "#audio.contents[10].freqs[0] = %g" % audio.contents[10].freqs[0]
print "#audio.contents[10].phases[0,1] = %g,%g" % (audio.contents[10].phases[0], audio.contents[10].phases[1])

SAMPLES = int (audio.mix_freq * audio.frame_size_ms / 1000)
print "#", SAMPLES

for t in range (SAMPLES):
  result = 0
  orig = 0
  for i in range (len (audio.contents[10].freqs)):
    f = audio.contents[10].freqs[i]
    smag = audio.contents[10].phases[i * 2]
    cmag = audio.contents[10].phases[i * 2 + 1]
    result += sin (t * f / audio.mix_freq * 2 * pi) * smag
    result += cos (t * f / audio.mix_freq * 2 * pi) * cmag
  if (t < len (audio.contents[10].debug_samples)):
    orig = audio.contents[10].debug_samples[t]
  print result, orig
