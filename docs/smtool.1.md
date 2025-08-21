% smtool(1)	spectmorph-1 | SpectMorph Manual

# NAME

smtool - tool to show/change SpectMorph data from .sm/.smset files

# SYNOPSIS

**smtool** *`<sm_file>`{=html}\|`<smset_file>`{=html}* *`<command>`{=html}* \[ *`<command_specific_args>`{=html}* \]

# DESCRIPTION

**smtool** is a general purpose program that can perform operations on .sm/.smset files. Some of the operations extract data (like **size**), others modify data (like **auto-tune**). If a .smset file is given as argument, the operation is performed on the individual .sm files that are contained in the .smset file.

# OPTIONS

**smtool** options depend on the command used, the command specific arguments are listed below.

# COMMANDS

**volume** *`<percent>`{=html}*
:   Compute average energy of the audio data around *`<percent>`{=html}*.

```{=html}
<!-- -->
```

**fundamental-freq**
:   Output fundamental frequency of the audio data.

```{=html}
<!-- -->
```

**mix-freq**
:   Output mixing frequency (sample rate) of the audio data.

```{=html}
<!-- -->
```

**zero-values-at-start**
:   Output zero-values-at-start property of the audio data.

```{=html}
<!-- -->
```

**attack**
:   Output attack parameters of the audio data.

```{=html}
<!-- -->
```

**size**
:   Output overview of the size in bytes of the different parts of the audio data.

```{=html}
<!-- -->
```

**loop-params**
:   Output loop parameters of the audio data.

```{=html}
<!-- -->
```

**noise-params** *`<frame_no>`{=html}*
:   Output noise component of a frame of the audio data.

```{=html}
<!-- -->
```

**frame** *`<frame_no>`{=html}*
:   Show original samples and reconstructed samples for one frame of the audio data.

```{=html}
<!-- -->
```

**frame-params** *`<frame_no>`{=html}*
:   Show the freqencies/magnitudes of one frame of the audio data.

```{=html}
<!-- -->
```

**total-noise**
:   Sum up all noise parameters of all frames of the audio data.

```{=html}
<!-- -->
```

**nan-test**
:   Check all frame data contents for NaNs.

```{=html}
<!-- -->
```

**original-samples**
:   Show original samples of the audio data.

```{=html}
<!-- -->
```

**freq** *`<freq_min>`{=html}* *`<freq_max>`{=html}*
:   Show all frequency entries of the audio data in the specified range.

```{=html}
<!-- -->
```

**spectrum** *`<frame_no>`{=html}*
:   Compare original and reconstructed spectrum for a given frame.

```{=html}
<!-- -->
```

**auto-loop** *`<percent>`{=html}*
:   Set a one frame loop that starts and ends at the same point (given by *`<percent>`{=html}*).

```{=html}
<!-- -->
```

**tail-loop**
:   Loop audio data at the end using a one frame loop.

```{=html}
<!-- -->
```

**auto-tune**
:   Tune instrument using the (40%..60%) data to extract the actual frequency.

```{=html}
<!-- -->
```

**tune-all-frames**
:   Tune each frame of the instrument. This will discard minimal changes in frequency which are normal for most instruments, so it might sound artificial.

```{=html}
<!-- -->
```

**auto-volume** *`<percent>`{=html}*
:   Normalize audio volume, using the volume around *`<percent>`{=html}* as reference.

```{=html}
<!-- -->
```

**auto-volume-from-loop**
:   Normalize audio volume, using the volume of the looped part as reference.

# SEE ALSO {#see_also}

[smenc.1](smenc.1 "wikilink"),
[smplay.1](smplay.1 "wikilink")
