% smwavset(1)	spectmorph-1 | SpectMorph Manual

# NAME

smwavset - Program to manage spectmorph multiwave files

# SYNOPSIS

**smwavset** *`<command>`{=html}* \[*OPTIONS*\] \[ *`<command specific args>`{=html}\...* \]

# DESCRIPTION

**smwavset** is a command line tool to manage multi-wave files for SpectMorph. Ususally, a multi wave file is first initialized (using the **init** command), then different wave files are added using the **add** command. After this has been done, encoding all waves can be done with one single command, like decoding all waves.

# OPTIONS

**smwavset** follows the usual GNU command line syntax, with long options starting with two dashes (\'-\').

**-h**, **\--help**
:   Shows a brief help message.

```{=html}
<!-- -->
```

**-v**, **\--version**
:   Prints out smwavset version.

```{=html}
<!-- -->
```

**\--args** *`<arguments>`{=html}*
:   Specify additional arguments to be passed to smenc/smplay. This could be \--args \"-O1\" for encoding, or \--args \"\--no-sines\" for decoding.

```{=html}
<!-- -->
```

**-d**, **\--data-dir**
:   For encoding/decoding, this sets a data directory where the .wav files or a list of .wav files (or .sm files), it is important that different wavsets store their files in different directories (otherwise they will overwrite wav files that belong to a different wavset). So specifying a proper data directory which is different for each wavset is usually necessary. The default value is /tmp, which is only appropriate for testing; you should ensure that scripts setup a different value.

```{=html}
<!-- -->
```

**\--format** *`<field1>`{=html}*,*`<field2>`{=html}*,*`<field3>`{=html}*,\...,*`<fieldN>`{=html}*
:   Set the output format for the list command.

```{=html}
<!-- -->
```

**-j** *`<jobs>`{=html}*
:   Use *`<jobs>`{=html}* parallel jobs for encoding (for systems with more than one processor).

# COMMANDS

**init** \[ *`<options>`{=html}* \] *`<wavset>`{=html}*\...
:   Initializes a new wavset; can also initialize more than one wavset specified on the commandline.

```{=html}
<!-- -->
```

**add** \[ *`<options>`{=html}* \] *`<wavset>`{=html}* *`<midi_note>`{=html}* *`<path>`{=html}*
:   Adds a wave file to the wavset, where midi_note is the midi note the wave file has been recorded for.

```{=html}
<!-- -->
```

**list** \[ *`<options>`{=html}* \] *`<wavset>`{=html}*
:   Lists the wave files that are contained within the wavset. The output format for the list command can be specified using the format option (comma seperated fields). See FIELDS section for a list of valid fields

```{=html}
<!-- -->
```

**encode** \[ *`<options>`{=html}* \] *`<wset_filename>`{=html}* *`<smset_filename>`{=html}*
:   Encodes a wavset using smenc.

```{=html}
<!-- -->
```

**decode** \[ *`<options>`{=html}* \] *`<smset_filename>`{=html}* *`<wset_filename>`{=html}*
:   Decodes a wavset using smplay.

```{=html}
<!-- -->
```

**delta** \[ *`<options>`{=html}* \] *`<wset_filename1>`{=html}*\...*`<wset_filenameN>`{=html}*
:   Computes the difference between many wavsets. The differencce between the first and second wavset is the reference which is 100%. The other wavsets are measured against that reference.

```{=html}
<!-- -->
```

**link** *`<smset>`{=html}*
:   This command includes all .sm files that are referred to by the smset in one big file. This is useful as final step when building instruments.

# FIELDS

For the list command, valid fields are:

**midi-note**
:   Midi note of the audio entry (0-127)

```{=html}
<!-- -->
```

**channel**
:   Channel for the audio entry (0 is the first channel)

```{=html}
<!-- -->
```

**filename**
:   Filename of the audio entry

```{=html}
<!-- -->
```

**min-velocity**
:   Minimum velocity of the audio entry (0-127)

```{=html}
<!-- -->
```

**max-velocity**
:   Maximum velocity of the audio entry (0-127)

# SEE ALSO {#see_also}

[smenc.1](smenc.1 "wikilink"),
[smplay.1](smplay.1 "wikilink"),
[smstrip.1](smstrip.1 "wikilink")
