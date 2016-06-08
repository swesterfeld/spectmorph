#!/bin/bash

source ./test-common.sh

EXIT=0

for MODE in noise saw
do
  if [ "x$MODE" = "xnoise" ]; then
    $WHITENOISE > test-norm.txt
    $SMENC -f 440 --text-input-file 48000 --no-lpc --no-sines --no-attack test-norm.txt
  elif [ "x$MODE" = "xsaw" ]; then
    $SMENC -f 440 --no-attack --no-lpc -O1 $(infile_location saw440.wav) test-norm.sm
  else
    echo "bad MODE $mode"
    exit 1
  fi
  {
  echo "======== MODE $MODE ========"
  echo
  $SMPLAY test-norm.sm --det-random --rate 48000 --export test-norm.wav 2>&1
  ENERGY1=$($WAV2ASCII test-norm.wav | $AVG_ENERGY 5000 15000)
  echo "avg energy1 (db) = $ENERGY1"
  $SMTOOL test-norm.sm auto-volume 50 | sed 's/^/     | /'
  $SMPLAY test-norm.sm --det-random --rate 48000 --export test-norm.wav 2>&1
  ENERGY2=$($WAV2ASCII test-norm.wav | $AVG_ENERGY 5000 15000)
  echo "avg energy2 (db) = $ENERGY2"
  $SMTOOL test-norm.sm auto-volume 50 | sed 's/^/     | /'
  $SMPLAY test-norm.sm --det-random --rate 48000 --export test-norm.wav 2>&1
  ENERGY3=$($WAV2ASCII test-norm.wav | $AVG_ENERGY 5000 15000)
  echo "avg energy3 (db) = $ENERGY3"
  echo
  } > test-norm.log
  if [ "x$(echo $ENERGY1 $ENERGY2 $ENERGY3 | awk '{
    delta12 = $1 - $2
    if (delta12 < 0) { delta12 = -delta12 }

    delta23 = $2 - $3
    if (delta23 < 0) { delta23 = -delta23 }

    if (delta12 > 1.5 && delta23 < 1e-6)
      {
        print "OK"
      }
    else
      {
        print "FAIL"
      }
  }')" != "xOK" ]; then
    echo "FAIL test-norm-$MODE"
    echo
    cat test-norm.log |grep -v out.of.range
    EXIT=1
  else
    echo "OK test-norm-$MODE $ENERGY1 $ENERGY2 $ENERGY3"
  fi
done

rm test-norm.sm test-norm.log test-norm.wav test-norm.txt

exit $EXIT
