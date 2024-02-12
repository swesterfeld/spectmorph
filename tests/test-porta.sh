#!/bin/bash

source ./test-common.sh

set -e

EXIT=0

$TESTIFFTSYNTH portamento | $HILBERT 2000 > test-porta-live.txt
$TESTIFFTSYNTH portamento_sin | $HILBERT 2000 > test-porta-sin.txt
DIFFS=$(
paste test-porta-live.txt test-porta-sin.txt | awk '
  function abs(x) {
    return x < 0 ? -x : x;
  }
  function max(x,y) {
    if (x > y)
      return x;
    else
      return y;
  }
  BEGIN {
    fdiff = adiff = 0
  }
  {
    fdiff = max(fdiff, abs($4 - $8));
    adiff = max(adiff, abs($3 - $7));
  }
  END {
    if (fdiff > 0.1 && fdiff < 0.615 && adiff > 0.0001 && adiff < 0.0012)
      result = "OK";
    else
      result = "FAIL";

    print result, "test-porta", fdiff, adiff;
  }'
)
echo $DIFFS | grep FAIL && die "portamento test failed"
echo $DIFFS | grep OK || die "portamento test failed"

rm test-porta-live.txt test-porta-sin.txt

exit $EXIT
