#!/bin/bash

source ./test-common.sh

if [ "x$1" == "xwine" ]; then
  export WINE="wine"
  export WINEDEBUG="-all"
fi

smrunplan()
{
  $WINE ../tools/smrunplan "$@"
}

smfileedit()
{
  $WINE ../tools/smfileedit "$@" | tr -d '\r'
}

set -e pipefail

TESTS=0
TESTS_OK=0

make # need to build before calling list-refs to get clean output

echo === post install test ===
for PRESET in $(make list-refs | sed 's,ref/,,g;s,\.ref,,g')
do
  # we need to test without unison for the actual test, however we run in full unison
  # mode without validating the output to trigger asan/ubsan problems
  smrunplan --debug-in-test-program ../data/templates/$PRESET.smplan -q 2>/dev/null

  DISABLE_UNISON=$(smfileedit list ../data/templates/$PRESET.smplan |grep 'unison\[0\]=true' |sed 's/true/false/g')
  smfileedit edit ../data/templates/$PRESET.smplan test.smplan $DISABLE_UNISON

  smrunplan --debug-in-test-program --det-random test.smplan 2>/dev/null |
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
          ok = delta < 0.0025 ? "OK  " : "FAIL";
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
