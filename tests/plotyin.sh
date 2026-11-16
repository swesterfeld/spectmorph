#!/usr/bin/env bash
set -euo pipefail

interactive=false
if [[ ${1:-} == --interactive ]]; then
  interactive=true
  shift
fi

if [[ $# -lt 1 || $# -gt 4 || $1 == --help ]]; then
  cat <<'EOF'
Usage: tests/plotyin.sh [--interactive] samples.txt [pitch.png [expected-f0-hz [encoder-f0-hz]]]
       sample-generator | tests/plotyin.sh - pitch.png 100

Input: mono audio at 48000 Hz, one numeric sample per line (testyin format).
Output: a PNG plot and a matching .dat file containing the candidate probabilities.
Also runs the SpectMorph encoder and saves .encoder.dat with env_f0 converted to Hz.
Both estimates are plotted at their frame centers in the original audio.
Dot area represents probability; frequency zero denotes unvoiced probability.
Optional expected F0 enables a Gaussian audio low-pass before YIN analysis.
It sets the filter bandwidth, not the output pitch or a forced voicing decision.
Encoder F0 defaults to expected F0 if supplied, otherwise 440 Hz. The fourth
argument overrides it independently. Use '-' as expected F0 to disable filtering.
Requires a configured SpectMorph build and gnuplot with the pngcairo terminal.
--interactive also opens a Qt window: right-drag to zoom, 'u' to undo zoom.
Close the window to exit. Requires gnuplot's qt terminal and a desktop display.
EOF
  if [[ ${1:-} == --help ]]; then exit 0; else exit 1; fi
fi

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
input=$1
output=${2:-pitch.png}
yin_args=()
if [[ $# -ge 3 && $3 != - ]]; then
  yin_args=(--expected-f0 "$3")
fi
data="${output}.dat"
encoder_data="${output}.encoder.dat"
encoder_f0=${4:-${3:-440}}
if [[ $encoder_f0 == - ]]; then encoder_f0=440; fi
if [[ $input != - && ! -r $input ]]; then
  echo "Cannot read input: $input" >&2
  exit 1
fi
if [[ $input != - && ( $input -ef $output || $input -ef $data || $input -ef $encoder_data ) ]]; then
  echo "Input and output must be different files." >&2
  exit 1
fi

tmp_dir=$(mktemp -d)
trap 'rm -rf -- "$tmp_dir"' EXIT
# The comparison now links against SpectMorph's encoder.
build_args=()
if [[ -n ${CXX:-} ]]; then build_args+=("CXX=$CXX"); fi
make -C "$script_dir/../lib" "${build_args[@]}" libspectmorph.la >&2
make -C "$script_dir" "${build_args[@]}" testyin >&2
yin_args+=(--encoder-f0 "$encoder_f0" --encoder-output "$tmp_dir/encoder.dat")
if [[ $input == - ]]; then
  "$script_dir/testyin" "${yin_args[@]}" > "$tmp_dir/candidates.dat"
else
  "$script_dir/testyin" "${yin_args[@]}" < "$input" > "$tmp_dir/candidates.dat"
fi
if [[ ! -s $tmp_dir/candidates.dat ]]; then
  echo "No frames produced: testyin needs more than 3000 input samples." >&2
  exit 1
fi
gnuplot -c "$script_dir/plotyin.gp" "$tmp_dir/candidates.dat" "$tmp_dir/pitch.png" "$tmp_dir/encoder.dat"
cp -- "$tmp_dir/candidates.dat" "$data"
cp -- "$tmp_dir/pitch.png" "$output"
cp -- "$tmp_dir/encoder.dat" "$encoder_data"
printf 'Plot: %s\nCandidates: %s\nEncoder: %s\n' "$output" "$data" "$encoder_data"
if $interactive; then
  gnuplot -c "$script_dir/plotyin.gp" "$data" --interactive "$encoder_data"
fi
