#!/bin/bash

if [ "$#" -ne 4 ]; then
  echo "usage: testporta.sh <inst> <from> <to> <delta>"
  exit 1
fi

INST=$1
FROM=$2
TO=$3
DELTA=$4

END_SLIDE=$(python -c 'print 1.0 + '$DELTA)
END_TIME=$(python -c 'print 2.0 + '$DELTA)

(
echo "0.0 $FROM"
echo "1.0 $FROM"
echo "$END_SLIDE $TO"
echo "$END_TIME $TO"
) > /tmp/porta

set -x
smlive ~/.spectmorph/instruments/standard/${INST}.smset -f $FROM --loop $END_TIME --freq-in /tmp/porta -x /tmp/porta.wav
gst123 -a jack /tmp/porta.wav
