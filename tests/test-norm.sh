#!/bin/bash

source ./test-common.sh

for MODE in noise saw
do
  if [ "x$MODE" = "xnoise" ]; then
    $WHITENOISE > noise.txt
    $SMENC -f 440 --text-input-file 48000 --no-lpc --no-sines --no-attack noise.txt
  elif [ "x$MODE" = "xsaw" ]; then
    $SMENC -f 440 --no-attack --no-lpc -O1 $(infile_location saw440.wav) noise.sm
  else
    echo "bad MODE $mode"
    exit 1
  fi
  echo
  echo "======== MODE $MODE ========"
  echo
  $SMPLAY noise.sm --det-random --rate 48000 --export noise-out.wav
  $WAV2ASCII noise-out.wav | $AVG_ENERGY 5000 15000
  $SMTOOL noise.sm auto-volume 50 | sed 's/^/     | /'
  $SMPLAY noise.sm --det-random --rate 48000 --export noise-out.wav
  $WAV2ASCII noise-out.wav | $AVG_ENERGY 5000 15000
  $SMTOOL noise.sm auto-volume 50 | sed 's/^/     | /'
  $SMPLAY noise.sm --det-random --rate 48000 --export noise-out.wav
  $WAV2ASCII noise-out.wav | $AVG_ENERGY 5000 15000
done
echo
