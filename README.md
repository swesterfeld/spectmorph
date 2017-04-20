Compiling SpectMorph:
=====================

To compile SpectMorph, the usual

    ./configure
    make
    make install

needs to be used.

LV2 Support:
------------

Configure should automatically determine via pkg-config wether the lv2
development headers are available. When the LV2 plugin doesn't get built,
install them.

BEAST Support:
--------------

SpectMorph can be used together with BEAST (see
https://testbit.eu/wiki/Beast_Home).  Configure should automatically detect
whether to build the plugin or not.

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

BEAST Plugin:
=============

When built with BEAST support, SpectMorph has its own BEAST plugin. Since BEAST
is modular, it comes as an oscillator. So you can build your own instrument
network with SpectMorph generating the sound, and use the other components
from BEAST, like the amplifier and ADSR envelope.

JACK:
=====

The smjack program is a fully functional JACK Client using SpectMorph. You need
to connect midi input and audio output (for instance with using QJackCtl).

LV2 Plugin:
===========

SpectMorph provides a LV2 Plugin, which can be used together with DAWs that
support LV2, such as Ardour, Qtractor and others.

Ardour and other Hosts using SUIL:
----------------------------------

SUIL is a library that is used by Ardour (and probably others) to display the
plugin UI.  SpectMorph is using the Qt5 library for implementing its plugin UI.
The latest stable release of SUIL does not (at the time of writing this) come
with support for for Qt5. However the development version has support for Qt5,
(it was added on 2017-03-18). So you need a SUIL library version that is newer
than this, to be able to use SpectMorph in Ardour.

Crashes with Hosts using Qt4:
-----------------------------

Some Hosts have been compiled against the (old) Qt4 GUI library. Since
SpectMorph is using Qt5 for its code, loading the SpectMorph plugin into such a
host will create problems. Usually the Host will crash. I know of no workaround
for this issue other than building the Host against Qt5, too.

Qtractor:
---------

Qtractor (when compiled against Qt5) works fine with SpectMorph. However you
need to be aware that the SpectMorph plugin only generates mono output. For
Qtractor, by default the SpectMorph audio will only be played on the left
speaker. To fix this, you can add the Stereo Balance Control LV2 plugin right
after the mono instrument and then set channel assignemnt = 1 (L->L, L->R) ...

The plugin is available here:

    http://gareus.org/oss/lv2/balance

Controlling Morphing with Automation:
-------------------------------------

The LV2 Plugin has two properties that can be automated by the host, called
Control #1 and Control #2. To use these, for instance for linear morphing,
the Control Input can be set to "Control Signal #1" (or #2) in the UI. After
that the host can change the morphing from left source to right source and
back.

VST Plugin:
===========

The VST Plugin has been tested in renoise and bitwig-studio, and there it
should just work. In hosts that support LV2, using the SpectMorph LV2 plugin is
the preferred way of integration.

Controlling Morphing with Automation:
-------------------------------------
The VST Plugin has two properties that can be automated by the host, called
Control #1 and Control #2. To use these, for instance for linear morphing,
the Control Input can be set to "Control Signal #1" (or #2) in the UI. After
that the host can change the morphing from left source to right source and
back.
