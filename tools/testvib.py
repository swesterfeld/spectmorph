#!/usr/bin/env python2

import math
import os
import sys

freq = 6
d = 440 * 0.005

with open ("/tmp/testvib", "w") as freq_in:
  for i in range (5000):
    freq_in.write ("%.5f %.5f\n" % (i * 0.001, 440 + math.sin (freq * i * 0.001 * 2 * math.pi) * d))

os.system ("./smlive ~/.spectmorph/instruments/standard/%s.smset -f 440 --loop 5 --freq-in /tmp/testvib -x /tmp/testvib.wav" % sys.argv[1])
os.system ("gst123 -a jack /tmp/testvib.wav")
