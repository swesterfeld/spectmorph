% smjack(1)	spectmorph-1 | SpectMorph Manual

# NAME

smjack - SpectMorph JACK Client

# SYNOPSIS

**smjack** \[*morphplan.smplan*\]

# DESCRIPTION

**smjack** is a graphical JACK Client for creating/executing SpectMorph morph
plans. A morph plan describes how SpectMorph should morph data from different
sources. The plan (which consists of operators) can be interactively edited and
exported for later reuse. Passing no morph plan starts with an default plan,
whereas passing a morph plan loads this plan.

Once started, **smjack** creates JACK ports for midi input, control input and
audio output, which need to be connected using qjackctl or similar.

# OPTIONS

There are no options other than the optional morph plan.

# SEE ALSO {#see_also}

[smenc.1](smenc.1 "wikilink"),
[sminspector.1](sminspector.1 "wikilink")
