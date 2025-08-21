% smplay(1)	spectmorph-1 | SpectMorph Manual

# NAME

smplay - Program to play a SpectMorph model

# SYNOPSIS

**smplay** \[*OPTIONS*\] *SM-File\|SMSet-File*

# DESCRIPTION

**smplay** is a command line based player for SpectMorph models (which can be created using **smenc**). Playing can use the audio card, so that the model will be heard directly, or create a wav file instead if the **\--export** option is used.

Since the SpectMorph model consists of sine waves and noise, options to disable one of these components exist.

# OPTIONS

**smplay** follows the usual GNU command line syntax, with long options starting with two dashes (\'-\').

**-h**, **\--help**
:   Shows a brief help message.

```{=html}
<!-- -->
```

**-v**, **\--version**
:   Prints out **smplay** version.

```{=html}
<!-- -->
```

**\--rate** *`<sampling rate>`{=html}*
:   Set replay rate manually; also useful for specifying the desired rate of the wav file, if **\--export** is used.

```{=html}
<!-- -->
```

**\--no-noise**
:   Disable noise decoder, so that only the sine wave part of the signal is decoded.

```{=html}
<!-- -->
```

**\--no-sines**
:   Disable sine decoder, so that only the noise part of the signal is decoded.

```{=html}
<!-- -->
```

**\--det-random**
:   Use deterministic random generator; this will produce identical noise components if the same file is decoded twice, making the audio output exactly the same.

```{=html}
<!-- -->
```

**\--export** *`<wav filename>`{=html}*
:   Instead of playing the file, write the output to a wav file.

```{=html}
<!-- -->
```

**-m**, **\--midi-note** \'\'
:   Select midi note to play, in case an SMSet-File was specified as input file.

# SEE ALSO {#see_also}

[smenc.1](smenc.1 "wikilink")
