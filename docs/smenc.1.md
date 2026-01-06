% smenc(1)	spectmorph-1 | SpectMorph Manual

# NAME

smenc - Program to encode a wav file into a SpectMorph model

# SYNOPSIS

**smenc** \[*OPTIONS*\] *WAV-File* \[*SM-File*\]

# DESCRIPTION

**smenc** is a command line tool to create SpectMorph models from audio files.
Usually the input file should contain a single note of some instrument, like a
single C4 note. The midi note (or the fundamental frequency) of the sample is
required to perform the analysis, and can be provided using the -m or -f
option. Another possibility is to use -M or -F which tries to auto-detect the
midi note / fundamental frequency.

There is also a quality / time tradeoff, which means you can use the -O0, -O1 or -O2 option to specify how accurate
the analysis should be, with higher numbers meaning better quality, but longer analysis time.

For multi-channel inputs, each channel is encoded seperately. If you do not specify the output filename, the
encoder will for instance create piano-ch0.sm and piano-ch1.sm from the stereo file piano.wav. If you do specify
an output filename, you need to include a %c into the name, like piano-ch%c.sm, which will be substituted with the
channel number.

# OPTIONS

**smenc** follows the usual GNU command line syntax, with long options starting with two dashes (\'-\').

**-h**, **\--help**
:   Shows a brief help message.

**-v**, **\--version**
:   Prints out smenc version.

**-f** *`<frequency>`*
:   Specify fundamental frequency in Hz. This is equivalent to specifying the
midi note; only one of -m and -f should be used.

**-m** *`<midi-note>`*
:   Specify the midi note of the input; this will be used to set the
fundamental frequency in Hz. The midi note should be an integer between 0 and
127.  Only one of -m and -f should be used.

**-F**
:   Automatically detect the fundamental frequency of the input sample. This
works well for input material with a harmonic spectrum.

**-M**
:   Automatically detect the midi note of the input sample. This is similar to
the -F option, however the result of the frequency detection will be rounded to
the nearest midi note.

**-O** *`<level>`*
:   For all frames of the input file, once smenc has found out which sine waves
can be used to describe the frame, it needs to estimate the magnitude and phase
of each sine wave for this frame. The optimization level chooses the algorithm
used for finding the magnitude and phase; the higher the level, the more
accurate the result will be. However, it defaults to -O0, since this is the
fastest algorithm: estimating the magnitude/phase from the FFT result. A good
choice is -O1, which is somewhat slower, but also somewhat better.

**-s**
:   Right now, smenc defaults to including lots of information in the output
replaying the file. Therefore the files created by smenc are very large by
default. To limit the file size, using the -s option instruct smenc to create a
\"stripped\" model, that contains only the information needed to play it. These
stripped models are usually much smaller than unstripped models.

**\--no-attack**
:   By default, smenc tries to find an attack envelope at the beginning of the
input, which describes at which time point the attack occurs and how fast the
attack is. This option disables that step (which uses quite a bit of CPU time).

**\--no-sines**
:   Skip analysis of the sine part of the signal (partial tracking).

**\--loop-start**
:   Set start loop point (in samples) - loop type is set to timeloop.

**\--loop-end**
:   Set end loop point (in samples) - loop type is set to timeloop.

# SEE ALSO {#see_also}

[smplay.1](smplay.1 "wikilink")
