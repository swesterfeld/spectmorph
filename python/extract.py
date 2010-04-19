#!/usr/bin/env python

import sys
import spectmorph

try:
  audio = spectmorph.load_stwafile (sys.argv[1])
except:
  print "cannot load file:", sys.argv[1]

print audio
