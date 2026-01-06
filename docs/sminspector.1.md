% sminspector(1)	spectmorph-1 | SpectMorph Manual

# NAME

sminspector - program to visualize contents of SpectMorph instruments

# SYNOPSIS

**sminspector** *\[OPTIONS\]* *SMIndex-File*

# DESCRIPTION

sminspector is a gui application which can be used to visualize SpectMorph models. An input file with containing a
list of instruments to open should be passed as first argument. This file is a text file, consisting of the commands listed below, and specifies a set of instruments to work with.

# INDEX FILE COMMANDS {#index_file_commands}

**smset_dir** \"*directory*\"
:   Set directory which contains the smset files.

**smset** \"*instrument.smset*\"
:   Add smset-instrument.

# SEE ALSO {#see_also}

[smenc.1](smenc.1 "wikilink"), [smvisualize.1](smvisualize.1 "wikilink")
