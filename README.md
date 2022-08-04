[![License][lgpl2.1-badge]][lgpl2.1-url]
[![Test Build][testing-badge]][testing-url]

Compiling SpectMorph:
=====================

To compile SpectMorph, use the usual

    ./configure
    make
    make install

LV2 Support:
------------

Configure should automatically determine via pkg-config whether the lv2
development headers are available. When the LV2 plugin doesn't get built,
install them.

VST Plugin:
===========

The VST Plugin is available for Linux, macOS and 64-bit Windows, and should
work in many different hosts, such as Bitwig, Renoise, Cubase and others. For
Linux hosts that support LV2, using the SpectMorph LV2 plugin is the preferred
way of integration.

Controlling Morphing with Automation:
-------------------------------------
The VST Plugin has four properties that can be automated by the host, called
Control #1 ... Control #4. To use these, for instance for linear morphing,
the Control Input can be set to "Control Signal #1" (or #2) in the UI. After
that the host can change the morphing from left source to right source and
back.

LV2 Plugin:
===========

SpectMorph provides a LV2 Plugin, which can be used together with Linux DAWs
that support LV2, such as Ardour, Qtractor and others.

Controlling Morphing with Automation:
-------------------------------------

The LV2 Plugin has four properties that can be automated by the host, called
Control #1 ... Control #4. To use these, for instance for linear morphing,
the Control Input can be set to "Control Signal #1" (or #2) in the UI. After
that the host can change the morphing from left source to right source and
back.

JACK:
=====

The smjack program is a fully functional JACK Client using SpectMorph. You need
to connect midi input and audio output (for instance with using QJackCtl).

Controlling Morphing with CCs:
------------------------------
The control inputs (Control Signal #1 ... Control Signal #4) are mapped to the
midi CC controls (General Purpose Controller 1..4).

Packaging Instruments:
======================

If you are a packager, packages should install the standard instrument set to a
system-wide location. The instrument location is prefix dependant. If
SpectMorph is installed in

    /usr

then the instruments should go to

    /usr/share/spectmorph/instruments

If you are building from the original source tarball, this should work automatically,
the instruments are bundled and will be installed during

    $ make install

Installing Instruments for Git:
===============================

If you're building from git, you are responsible for obtaining a copy of the standard
instrument tarball. Extract the SpectMorph release source tarball. The location will
be something like

    spectmorph-0.4.1/data/spectmorph-instruments-0.4.1.tar.xz

You can either ensure that these instruments live in (`$XDG_DATA_HOME/spectmorph`):

    ~/.local/share/spectmorph/instruments/standard

or copy the instrment tarball to the data directory of your git checkout. In this
case SpectMorph will automatically install the instruments to the system-wide
location (like the original release tarball does).

Note that if incompatible changes happen to the file format, the git version will
no longer work with the released instruments set. In that case, feel free to ask
for a tarball for your git version.

[lgpl2.1-badge]: https://img.shields.io/github/license/swesterfeld/liquidsfz?style=for-the-badge
[lgpl2.1-url]: https://github.com/swesterfeld/liquidsfz/blob/master/COPYING
[testing-badge]: https://img.shields.io/github/workflow/status/swesterfeld/spectmorph/Testing?style=for-the-badge
[testing-url]: https://github.com/swesterfeld/spectmorph/actions/workflows/testing.yml
