#!/usr/bin/env python

import sys
import spectmorph

try:
  audio = spectmorph.load_stwafile (sys.argv[1])
except:
  print "cannot load file:", sys.argv[1]

print "Audio File:"
print "fundamental_freq = %f Hz" % audio.fundamental_freq
print "mix_freq = %f Hz" % audio.mix_freq
print "frame_size_ms = %f ms" % audio.frame_size_ms
print "frame_step_ms = %f ms" % audio.frame_step_ms
print "zeropad = %d" % audio.zeropad
