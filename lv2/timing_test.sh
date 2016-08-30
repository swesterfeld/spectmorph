#!/bin/bash

# detect note on from audio: debug midi event -> audio timing
wav2ascii $1 | awk '
  BEGIN {
    lastn=0;
  }
  {
    # note on detected if value != 0 after at leas 16 zeros
    if ($1 != 0 && zeros > 16)
      {
        print n, n - lastn;

        zeros = 0;
        lastn = n
      }
    # count number of zero samples
    if ($1 == 0)
      {
        zeros++;
      }
    else
      {
        zeros = 0;
      }
    n++;
  }'
