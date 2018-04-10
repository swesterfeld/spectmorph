Compiling SpectMorph:
=====================

To compile SpectMorph, the usual

    ./configure
    make
    make install

needs to be used.

LV2 Support:
------------

Configure should automatically determine via pkg-config whether the lv2
development headers are available. When the LV2 plugin doesn't get built,
install them.

Installing Instruments:
=======================

To use SpectMorph, you need to install the standard Instrument Set first.
SpectMorph will load instruments automatically from your home directory.
The convention is that you install them in

    ~/.spectmorph/instruments

All UIs (BEAST|JACK|LV2|VST) will warn you with the message

    Instrument Set 'standard' NOT FOUND.

If the instruments could not be loaded. In this case standard is the name
of the instrument set that is missing. It would be searched in

    ~/.spectmorph/instruments/standard

The tarball containing the standard instruments can be downloaded from
http://spectmorph.org/downloads and should be extracted with these three steps:

    $ mkdir ~/.spectmorph
    $ cd ~/.spectmorph
    $ tar xvf /path/to/downloaded/instruments.tar.xz

This should put the instruments into the correct directory. After this, all
templates that can be loaded from the UI (and the default template) should
produce sound out of the box.

Packaging Instruments:
======================

If you are a packager, packages can install the instruments to a system-wide
location. That way users don't need to download the instruments seperately.
The instrument location is prefix dependant. If SpectMorph is installed in

    /usr

then the instruments should go to

    /usr/share/spectmorph/instruments

VST Plugin:
===========

The VST Plugin is available for Linux and 64-bit Windows, and should work in
many different hosts, such as Bitwig, Renoise, Cubase and others. For Linux
hosts that support LV2, using the SpectMorph LV2 plugin is the preferred way of
integration.

Controlling Morphing with Automation:
-------------------------------------
The VST Plugin has two properties that can be automated by the host, called
Control #1 and Control #2. To use these, for instance for linear morphing,
the Control Input can be set to "Control Signal #1" (or #2) in the UI. After
that the host can change the morphing from left source to right source and
back.

LV2 Plugin:
===========

SpectMorph provides a LV2 Plugin, which can be used together with Linux DAWs
that support LV2, such as Ardour, Qtractor and others.

Controlling Morphing with Automation:
-------------------------------------

The LV2 Plugin has two properties that can be automated by the host, called
Control #1 and Control #2. To use these, for instance for linear morphing,
the Control Input can be set to "Control Signal #1" (or #2) in the UI. After
that the host can change the morphing from left source to right source and
back.

JACK:
=====

The smjack program is a fully functional JACK Client using SpectMorph. You need
to connect midi input and audio output (for instance with using QJackCtl).

BEAST Plugin:
=============

Previous versions of SpectMorph included a BEAST plugin. Starting with
SpectMorph 0.3.4, the plugin code has been removed from SpectMorph. Instead,
new versions of BEAST will include the SpectMorph plugin, and new binary
releases of BEAST should include SpectMorph, so as user once this integration
is completed, SpectMorph should work out of the box in BEAST. See

    https://github.com/tim-janik/beast/issues/12

Since BEAST is modular, the SpectMorph plugin comes as an oscillator. So you
can build your own instrument network with SpectMorph generating the sound, and
combine SpectMorph with the other components from BEAST.


