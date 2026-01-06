% smstrip(1)	spectmorph-1 | SpectMorph Manual

# NAME

smstrip - program to remove extra data from SpectMorph model files

# SYNOPSIS

**smstrip** *SM-File\...*

# DESCRIPTION

**smstrip** is a tool which removes all debugging data from a SpectMorph model.
By default, smenc includes more data than is required for playback in the SM-File.
This extra data can be removed, thereby shrinking the file size considerably.
Note that stripped models can be created directly by the encoder using **smenc -s**.

# OPTIONS

**smstrip** follows the usual GNU command line syntax, with long options starting with two dashes (\'-\').

**-h**, **\--help**
:   Shows a brief help message.

**-v**, **\--version**
:   Prints out **smstrip** version.

**\--keep-samples**
:   Strips debugging information, but keeps the original sample data (useful for quality comparisions). This of course reduces the file size less than full stripping.

# SEE ALSO {#see_also}

[smenc.1](smenc.1 "wikilink"),
[smplay.1](smplay.1 "wikilink")
