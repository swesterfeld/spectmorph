# Usage: gnuplot -c tests/plotyin.gp candidates.dat pitch.png [encoder.dat]
#        gnuplot -c tests/plotyin.gp candidates.dat --interactive [encoder.dat]
# Input columns from testyin: sample offset, frequency in Hz, probability.
# testyin currently assumes a 48000 Hz sample rate.
interactive = (ARG2 eq "--interactive")
has_encoder = (ARGC >= 3)
encoder_file = has_encoder ? ARG3 : ARG1
# Candidate rows use frame starts; testyin's 2999-sample support centers at +1499.
yin_time(start) = (start + 1499) / 48000.0
if (interactive) {
    set terminal qt size 1400,800 enhanced font "Sans,12" title "YIN — right-drag to zoom, u to undo"
    set mouse
} else {
    set terminal pngcairo size 1400,900 enhanced font "Sans,12"
    set output ARG2
}
set datafile separator whitespace
stats ARG1 using (yin_time($1)) nooutput
set xrange [STATS_min:STATS_max]
set y2range [0:1.05]
if (interactive) {
    # A single plot keeps mouse zoom working on pitch and probability together.
    set title "Probabilistic YIN candidates — dot area represents probability"
    set y2tics 0.25
    set y2label "Unvoiced probability"
} else {
    set multiplot layout 2,1 title "Probabilistic YIN candidates — dot area represents probability"
}
set lmargin 10
set rmargin 4
if (interactive) { set rmargin 10 }
set grid xtics ytics back lc rgb "#dddddd"
set key outside top center horizontal
set ylabel "Frequency (Hz)"
set logscale y 2
set yrange [32:2048]
set ytics ("32" 32, "64" 64, "128" 128, "256" 256, "512" 512, "1024" 1024, "2048" 2048)
unset xlabel
if (interactive) { set xlabel "Frame center time (seconds)" }
# Point size scales diameter: sqrt(probability) makes area proportional to it.
plot ARG1 using (yin_time($1)):($2 > 0 && $3 > 0 ? $2 : 1/0):(2.5 * sqrt($3)) \
       with points pt 7 ps variable lc rgb "#2369a1" title "Pitch candidates", \
     NaN with points pt 7 ps 2.5 lc rgb "#2369a1" title "p = 1", \
     NaN with points pt 7 ps 1.25 lc rgb "#2369a1" title "p = 0.25", \
     NaN with points pt 7 ps 0.5 lc rgb "#2369a1" title "p = 0.04", \
     ARG1 using (yin_time($1)):(interactive && $2 == 0 ? $3 : 1/0) axes x1y2 \
       with lines lw 1.5 lc rgb "#c46b27" title (interactive ? "Unvoiced (right axis)" : ""), \
     for [i=1:has_encoder] encoder_file using ($1 / 48000.0):($2 > 0 ? $2 : 1/0) \
       with lines lw 2 lc rgb "#b52b62" title "Encoder env\\_f0 (Hz)"
if (interactive) {
    # Keep gnuplot running so zoom can replot from the saved candidate data.
    pause mouse close
    exit
}
unset logscale y
set yrange [0:1.05]
set ytics 0.25
set ylabel "Unvoiced probability"
set xlabel "Frame center time (seconds)"
unset key
plot ARG1 using (yin_time($1)):($2 == 0 ? $3 : 1/0) \
       with linespoints pt 7 ps 0.4 lw 1.5 lc rgb "#c46b27"
unset multiplot
