#!/bin/bash
for mode in tune-all-frames auto-tune
do
  smenc -f 420 --no-attack --no-lpc -O1 saw440.wav tune-test.sm
  smtool tune-test.sm $mode
  if [ "x$(smtool tune-test.sm frame-params 30 | head -5 | awk '
    BEGIN {
      result="OK"
    }
    {
      delta = (NR * 420 - $1) / NR
      if (delta < 0) { delta = -delta } # abs value
      if (delta > 0.1) {
        result = "FAIL"
      }
    }
    END {
      print result
    }')" != "xOK" ]
  then
    echo "tune-test failed, mode $mode"
    smtool tune-test.sm frame-params 30 | head -5
    exit 1
  fi
done
rm tune-test.sm
exit 0
