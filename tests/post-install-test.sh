#!/bin/bash

source ./test-common.sh

TESTS=0
TESTS_OK=0

echo === post install test ===
for PRESET in $(make list-refs | sed 's,ref/,,g;s,\.ref,,g')
do
  ../tools/smrunplan --debug-in-test-program --det-random ../data/templates/$PRESET.smplan 2>/dev/null |
    ../tests/avg_energy.py -1 -1 > $PRESET.tmp

  if ! test -f ref/$PRESET.ref; then
    echo missing ref/$PRESET.ref, to fix use
    echo mv $PRESET.tmp ref/$PRESET.ref
  else
    RESULT=$(
      echo $PRESET $(cat ref/$PRESET.ref) $(cat $PRESET.tmp) | awk '
        function abs(x) {
          return x < 0 ? -x : x
        }
        function max(x, y) {
          return x > y ? x : y
        }
        {
          base = 1;
          base = max($2, base);
          base = max($3, base);
          delta = abs($2 - $3);
          ok = delta < 0.0001 ? "OK  " : "FAIL";
          printf ("%s %.5f %s\n", ok, abs($2 - $3) / base, $1);
        }'
    )
    echo "$RESULT"
    [[ $RESULT = OK* ]] && TESTS_OK=$((TESTS_OK + 1)) && rm $PRESET.tmp
    TESTS=$((TESTS + 1))
  fi
done
echo === $TESTS_OK/$TESTS tests passed ===
[[ $TESTS != $TESTS_OK ]] && die "post install test failed"
exit 0
