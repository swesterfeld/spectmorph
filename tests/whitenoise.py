#!/usr/bin/env python3

import random

# for deterministic tests
random.seed (42)
for i in range (20000):
  print (random.uniform (-0.5, 0.5))
