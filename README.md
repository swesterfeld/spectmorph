[![License][lgpl2.1-badge]][lgpl2.1-url]
[![Test Build][testing-badge]][testing-url]
[![Version][version-badge]][version-url]

About:
======

SpectMorph is a free software project which allows to analyze samples of
musical instruments, and to combine them (morphing). It can be used to
construct hybrid sounds, for instance a sound between a trumpet and a flute; or
smooth transitions, for instance a sound that starts as a trumpet and then
gradually changes to a flute. In its current version, SpectMorph ships with
many ready-to-use instruments which can be combined using morphing.

* The main website for this project is:
    https://www.spectmorph.org

Compiling SpectMorph:
=====================

To compile SpectMorph, use the usual

    ./configure
    make
    make install

If you get a message that the instruments are missing, you can use the
`--with-download-instruments` configure option to fix this (see below).

LV2 Support:
------------

Configure should automatically determine via pkg-config whether the lv2
development headers are available. When the LV2 plugin doesn't get built,
install them.

Plugins:
========

SpectMorph supports three different plugin formats,

 * CLAP plugin
 * VST plugin
 * LV2 plugin

for Linux, macOS (x86_64 and Apple Silicon) and 64-bit Windows, so it works in
many different hosts, such as Ardour, Qtractor, Anklang, Bitwig, Renoise,
Cubase, Ableton Live and others. For hosts that support CLAP, using the
SpectMorph CLAP plugin is the preferred way of integration.

Controlling Morphing with Automation:
-------------------------------------
The plugin has four properties that can be automated by the host, called
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

SpectMorph Instruments:
=======================

SpectMorph has a set of instruments which ship with official releases. These
are required to use SpectMorph. The instruments are included in all binary
releases. If you are building from the original source tarball, the
instruments are also included (in the `data` directory).

If you're building from git, you will need to download the instruments before
installing SpectMorph. The easiest way to do so is using:

    $ ./autogen.sh --with-download-instruments

which will download the correct version of instruments automatically. You can
also download the appropriate version of the instruments from the releases of
the `spectmorph-instruments` git repository and store them in the `data`
directory before building SpectMorph:

* [Instrument Releases](https://github.com/swesterfeld/spectmorph-instruments/releases)
* [Instrument Repository on Github](https://github.com/swesterfeld/spectmorph-instruments)

[lgpl2.1-badge]: https://img.shields.io/github/license/swesterfeld/spectmorph?style=for-the-badge
[lgpl2.1-url]: https://github.com/swesterfeld/spectmorph/blob/master/COPYING
[testing-badge]: https://img.shields.io/github/actions/workflow/status/swesterfeld/spectmorph/testing.yml?style=for-the-badge
[testing-url]: https://github.com/swesterfeld/spectmorph/actions/workflows/testing.yml
[version-badge]: https://img.shields.io/github/v/release/swesterfeld/spectmorph?label=version&style=for-the-badge
[version-url]: https://github.com/swesterfeld/spectmorph/releases
