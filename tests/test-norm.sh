#!/bin/bash

SMENC=../src/smenc
SMPLAY=../src/smplay
SMTOOL=../src/smtool

for MODE in noise saw
do
  if [ "x$MODE" = "xnoise" ]; then
    whitenoise.py > noise.txt
    $SMENC -f 440 --text-input-file 48000 --no-lpc --no-sines --no-attack noise.txt
  elif [ "x$MODE" = "xsaw" ]; then
    $SMENC -f 440 --no-attack --no-lpc -O1 saw440.wav noise.sm
  else
    echo "bad MODE $mode"
    exit 1
  fi
  echo
  echo "======== MODE $MODE ========"
  echo
  $SMPLAY noise.sm --det-random --rate 48000 --export noise-out.wav
  wav2ascii noise-out.wav | avg_energy.py 5000 15000
  $SMTOOL noise.sm auto-volume 50 | sed 's/^/     | /'
  $SMPLAY noise.sm --det-random --rate 48000 --export noise-out.wav
  wav2ascii noise-out.wav | avg_energy.py 5000 15000
  $SMTOOL noise.sm auto-volume 50 | sed 's/^/     | /'
  $SMPLAY noise.sm --det-random --rate 48000 --export noise-out.wav
  wav2ascii noise-out.wav | avg_energy.py 5000 15000
done
echo
