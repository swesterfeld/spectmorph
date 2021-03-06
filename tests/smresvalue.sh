#!/bin/bash

source test-common.sh

SMENCFLAGS="--no-attack"

WAV=$(infile_location "$1")
START="$2"
STOP="$3"
RATE="$4"
FREQ="$5"
FMT="$(echo $1 | sed 's/.*[.]//g')"

test -z "$FREQ" && die "freq parameter missing"

SM=$(echo "$1" | sed s/.$FMT$/.sm/g)
DWAV=$(echo "$1" | sed s/.$FMT$/-dec.wav/g)
TWAV=$(echo "$1" | sed s/.$FMT$/.txt/g)
TDWAV=$(echo "$1" | sed s/.$FMT$/-dec.txt/g)
TDEBUGDEC=$(echo "$1" | sed s/.$FMT$/-debug-dec.txt/g)

for ENCOPT in 0 1
do
  if [ "x$FMT" = "xwav" ]; then
    $SMENC $SMENCFLAGS -O$ENCOPT -f $FREQ $WAV $SM --debug-decode $TDEBUGDEC || die "smenc failed"
    $WAV2ASCII $WAV > $TWAV
  else
    $SMENC $SMENCFLAGS -O$ENCOPT -f $FREQ --text-input-file $RATE $WAV $SM --debug-decode $TDEBUGDEC || die "smenc failed"
    # $WAV and $TWAV contain the same filename
  fi

  $SMPLAY $SM --no-noise --rate=$RATE --export $DWAV || die "smplay failed"
  $WAV2ASCII $DWAV > $TDWAV
  SN=$($SN_DELTA $TWAV $TDEBUGDEC $START $STOP 70) || exit 1
  SN_QUANT=$($SN_DELTA $TWAV $TDWAV $START $STOP 60) || exit 1
  printf "%-25sS/N value:   %7s      quant   %7s   -O$ENCOPT\n" $1 $SN $SN_QUANT
done
rm $SM $TWAV $TDWAV $DWAV $TDEBUGDEC
