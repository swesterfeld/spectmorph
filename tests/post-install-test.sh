#!/bin/bash

source ./test-common.sh
if [ "x$( ../tools/smrunplan --debug-in-test-program ../data/templates/1-instrument.smplan | ../tests/avg_energy.py -1 -1 | awk '{ print ($1 > -50) ? "OK" : "FAIL" }')" != "xOK" ]; then
  echo "Post Install Test: FAIL"
  exit 1
else
  exit 0
fi
