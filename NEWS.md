## SpectMorph NEXT

#### New features
* Support click & drag in instrument editor sample to scroll & zoom (#22).
* Support stereo to mono conversion for files added in instrument editor (#14).
* Support multiple banks for wav sources / instrument editor.

#### Fixes
* Fix crash if instrument editor is closed without any samples

#### Internals: Optimizations
* Significantly reduce number of allocations done in DSP thread

## SpectMorph 0.6.0

#### New features
* New, more flexible modulation system
* Added filter with different filter modes
* Provide visual feedback for modulated properties
* Provide signed .pkg installers for macOS (Intel and ARM)

#### CLAP Plugin
* Provide CLAP Plugin
* Support for per-voice modulation
* Support timestamped modulation/automation events

#### LV2 Plugin
* Support LV2 on all platforms
* Fix crashes triggered by Carla (absolute_path/abstract_path returning NULL)
* Support newer LV2 development headers

#### Minor Changes
* Support "Velocity" as modulation source
* Make pitch bend range configurable
* New Presets with filter: "Cheese Cake Bass", "Liquid Silver"
* Sort midi events by timestamp to workaround Bitwig bug
* Sliders now support shift+drag for fine editing
* Support for Apple Silicon
* Avoid crashes if XOpenIM / XCreateIC return NULL (#15).
* Fix statically linked plugin data directory location (works in flatpak apps now).
* Add dockerized MXE builds for windows, bump compiler version to gcc-12.
* Bump minimum C++ standard to C++17
* Fix build on RISC-V (#13)
* Use GitHub CI for Linux, macOS and Windows
* Change license from "LGPL v3 or later" to "LGPL v2.1 or later".
* Minor fixes and cleanups

#### Internals: Properties
* Add generic property handling
* Simplify load/save/gui for properties
* Support modulatable properties using ModulationList
* Add gui for editing property value and ModulationList
* MorphPlan is no longer ref-counted, just one instance per Project
* Introduced MorphOperatorConfig objects for cleaner/faster parameter updates

#### Internals: Filter
* Add two filter types: "Ladder" and "Sallen-Key" filter to output operator
* Integrated PandaResampler for SIMD 4x filter oversampling
* Support modulation with high time resolution for filter

#### Internals: UI Toolkit
* Support multiple update regions in UI toolkit
* Optimize drawing for UI toolkit
* Support "software sprites" for efficient visual feedback
* Map Ctrl+Left Click to Right Click on macOS

#### Internals: Optimizations
* Pass wav set pointers (instead of strings) to morph linear/grid/source.
* Avoid fmod() for phase truncation.
* Build using -ffast-math
* NotifyBuffer: fast dsp thread -> ui thread notifications (no malloc in dsp thread)
* Avoid allocations in dsp thread in many cases (retrigger, noise decoder process)
* Support optimized SIMD code on ARM (Apple Silicon), code from Peter Johnson (#11)

## SpectMorph 0.5.2

* Support bpm/beat synchronization for LFO
  - new presets using beat sync LFO: Mars / Saturn
* Add WavSource custom position playback mode
* New Instruments: Sven Ah / Ih / Oh (another male human voice)
* Store data in XDG directories on Linux:
  - move ~/.spectmorph directory to $XDG_DATA_HOME/spectmorph
  - move ~/SpectMorph directory to $XDG_DOCUMENTS_DIR/SpectMorph
  - create $XDG_DOCUMENTS_DIR/SpectMorph directory when needed (on write)
  - backward compatibility: use ~/SpectMorph if it already exists
* Bump number of control inputs from 2 to 4
* Implemented midi CC control for smjack (General Purpose Controller 1..4)
* Fix crashes caused by dangling MorphOperator pointers
* Fix loading floating point wav files
* Minor fixes and cleanups

## SpectMorph 0.5.1

* Add new LFO modes (saw, square, random)
* Support generic 64-bit linux binaries
  - new linux file selector (no longer needs Qt)
  - ship font for static build
* Fix crashes caused by fftw planner being used from multiple threads
* Ported all python2 code to python3
* Support midi all notes off
* Implement LV2 StateChanged
* French translation for smjack desktop file (Olivier Humbert)
* Thread race fix (JP Cimalando)
* Minor fixes and cleanups

## SpectMorph 0.5.0

* Support user defined instruments
  - graphical instrument editor
  - new WavSource operator
* Make standard instrument set smaller (less download/disk usage)
* Graphical ADSR editor
* Added "SpectMorph User Manual" (online: html/pdf)
* Use different colors for active/used/unused operators
* LV2 now requires instance access feature
* Add file dialog wrapper shell scripts to work with some ardour bundles
* Integrate XML (pugixml) and ZIP (minizip) 3rd party code
* Minor fixes and cleanups

## SpectMorph 0.4.1

* macOS is now supported: provide VST plugin for macOS >= 10.9
* Include instruments in source tarball and packages
* Install instruments to system-wide location
* New Instruments: Claudia Ah / Ih / Oh (female version of human voice)
* Improved tools for instrument building
  - support displaying tuning in sminspector
  - implement "smooth-tune" command for reducing vibrato from recordings
  - minor encoder fixes/cleanups
  - smlive now supports enable/disable noise
* VST plugin: fix automation in Cubase (define "effCanBeAutomated")
* UI: use Source A / Source B instead of Left Source / Right Source
* UI: update db label properly on grid instrument selection change
* Avoid exporting symbols that don't belong to the SpectMorph namespace
* Fix some LV2 ttl problems
* Fix locale related problems when using atof()
* Minor fixes and cleanups

## SpectMorph 0.4.0

* Windows is now supported: provide 64-bit Windows VST plugin
* Plugin UI redesign
  - use pugl library for portability (uses GL + cairo) instead of Qt5
  - use categories for instruments
  - directly support instrument names in linear morphing
  - get rid of Qt5 dependency for libspectmorph, smjack, VST and LV2 plugins
  - UI now has "Zoom" feature to support higher DPI displays
* Use non-linear configurable new velocity -> volume mapping for midi
* New instrument: French Horn
* Improved tools for building custom instruments
  - tools are now installed by default
  - sminstbuilder files support new syntax for relative paths
  - encoder cache moved to ~/.cache/smenccache, which is created if necessary
  - use number of processors as default for jobs
* LPC/LSF support removed
* Some portability fixes for macOS (which however isn't supported yet)

## SpectMorph 0.3.4

* Added optional ADSR Envelope
* Make LV2 and VST plugin stereo to allow supporting stereo in the future
* LV2 plugin description fixes
* Added about dialog to plugin/smjack UI
* Remove BEAST plugin (plugin code will be moved to BEAST)
* Fixed compilation for newer g++ >= 6 (std::fabs)
* Get rid of some malloc() calls in linear morphing

## SpectMorph 0.3.3

* Added portamento:
  - VST: support MPE to perform per-voice pitch bend (can be used in Bitwig)
  - new portamento mono mode (all hosts)
* Added vibrato.
* Internal improvements:
  - better property abstraction for (non-linear) UI properties
  - updated polyphase interpolator (used for vibrato|portamento)
  - fixed a few problems when developing against spectmorph(ui) libs
  - don't link against Qt UI library when only QtCore is necessary
* Compile fixes for g++-6.3

## SpectMorph 0.3.2

* Added new unison effect.
* New instruments: pan-flute, synth-saw.
* UI improvements:
  - support operator folding (to preserve screen space)
  - provide scrollbar if morph plan window height is large
  - repair operator move
* VST plugin crash fixed.
* No longer depend on BEAST/Rapicorn
  - use libsndfile for sound file I/O, added WavData API
  - refactoring, move libnobse code into SpectMorph
* Add icon/.desktop file for smjack
* Added debian package support.
* LPC/LSF morphing code updates - but now disabled by default

## SpectMorph 0.3.1

* Added plugins for LV2 and VST api.
* New instruments: bassoon, cello, bass-trombone, reed-organ.
* Added different templates to get standard morph plans quickly.
* LV2|VST|JACK will start with default plan now (instead of empty plan).
* Standard instrument set location (~/.spectmorph/instruments/standard):
  - plan templates can refer to instruments in that directory without
    storing any absolute path (index will be instruments:standard)
  - in almost any case, loading instruments isn't necessary anymore
* Resize MorphPlanWindow automatically if operators are removed.
* Changed time alignment during morphing:
  - morphed sounds should starty at the beginning of the note (no extra latency)
  - Start marker for instrument notes no longer necessary
  - SpectMorphDelay plugin no longer necessary
* Some improvements for building new instruments:
  - make some smenc parameters configurable (--config option)
  - improvements to soundfont import
  - new fundamental frequency estimation for tune-all-frames
  - support global volume adjustment (instead of auto-volume)
* Various bugfixes.

## SpectMorph 0.3.0

* Incompatible file format changes:
  - use 16-bit integer values for sine and noise data instead of floats
    to reduce file size on disk and in memory
  - introduce short_name & name field for instruments
  - changed the way noise is represented from total band energy to
    normalized noise level
* Use Qt5 for the GUI, instead of gtkmm.
* Added Grid Morph operator: allows morphing between more than two sources.
* Adapted code to work with newer beast (0.10.0) and rapicorn (0.16.0).
* Improved tests.
* Various bugfixes.
* Performance improvements.

## SpectMorph 0.2.0

* implemented user defined morphing using a MorphPlan consisting of operators
  - graphical representation of the operators
  - graphical editing of the MorphPlan
  - implement actual morphing (in per-operator per-voice module object)
  - added MorphPlanSynth/MorphPlanVoice, which allow running MorphPlan's easily
  - added LPC (linear prediction) during encoding, and LPC/LSF based morphing
* BEAST plugin:
  - added GUI required for editing a MorphPlan
  - support four output channels, as well as twc control inputs
  - delay compensation plugin (to compensate SpectMorph delay)
* JACK client:
  - support GUI MorphPlan editing
* added sminspector (graphical tool for displaying SpectMorph instruments)
  - zoomable time/frequency view
  - configurable (FFT/CWT/LPC) time/frequency view transform parameters
  - spectrum, sample, LPC visualization
  - graphical loop point editing
  - allow storing changes in .smset files (for editing loop points)
  - play support via JACK
* improved smtool (old name: smextract); its now installed by default
  - lots of new commands (like "total-noise", "auto-volume", ...)
  - support .smset as input (in addition to .sm); command is executed on all
    .sm files in the .smset
* added shared libraries for gui and jack code
* new integrated memory leak debugger (to find missing delete's)
* support ping-pong loops
* doxygen API docs updates
* migrated man pages from Doxer to testbit.eu wiki (and use wikihtml2man.py)
* performance improvements

## SpectMorph 0.1.1

* added tool for SoundFont (SF2) import: smsfimport
* file format changes
  - allow time index based loops (required for looped SoundFont presets)
  - allow stereo (multichannel) files in WavSets
  - store phase and magnitude seperately (instead of a sin+cos magnitude)
  - support multiple velocity layers
  - allow storing the original sample data for quality comparisions
  - optimize storage size for smset files if the same Audio file is reused more
    than once
* switch to 32 values for 32 perceptually spaced noise bands, instead of the old
  noise representation
* handle stereo (multichannel) files in smenc, smjack and beast plugin
* performance optimizations
  - LiveDecoder is now really fast, and can handle highly polyphonic synthesis in RT
  - sine synthesis is based on IFFT now
  - noise synthesis is a lot faster, too
  - where possible, use SSE operations in performance critical code
  - use FFTW for FFT, which is faster than gslfft
  - added fast float->int conversion on x86
  - smjack is a lot faster, now
* removed smenc -O2 setting, which was too slow for practical use
* introduced anti-alias filter in LiveDecoder
* cleanups, refactoring, bugfixes

## SpectMorph 0.1.0

* file format changes
  - instruments based on more than one sample can be shipped as one single file
  - various performance optimizations
  - store data as little endian (since this is more likely to be the host endianness)
  - broken files or old files can be recognized and rejected
* automated tuning algorithm (smextract auto-tune)
* supported looping (for playing notes that is longer than original sample)
* added beast plugin for playing SpectMorph instruments
* added jack client for playing SpectMorph instruments
* added zero padding before start of a sample to get better initial frames
* compile with -Wall
* allow single file argument for smenc (output filename will be constructed with .sm extension)
* support setting smplay decoder mode via command line parameter
* refactoring, cleanups

## SpectMorph 0.0.3

* added encoder algorithm to find attack envelope, this makes piano sound much
  more realistic
* introduced smwavset tool, which allows managing instruments consisting of
  many samples
  - encoding/decoding a set of samples
  - delta operation for comparing errors of sets of samples
* smextract can now provide an overview of how many bytes in an .sm file can be
  attributed to which fields
* documentation updates
* refactoring, cleanups

## SpectMorph 0.0.2

* bugfixes
* include proper phases, so phase-correct reconstruction of samples is possible
* new programs:
  - smstrip     - removes debugging information from SpectMorph model files
  - smextract   - extracts data from SpectMorph models, for developers only for now
* added --no-noise / --no-sines switches for smplay
* added -s switch for smenc, to create stripped models
* use boost numeric bindings + lapack for ideal phase/magnitude estimation (smenc -O2) - slow!
* SSE optimizations and other speedups for fast phase/magnitude estimation (smenc -O1)
* use odd/centered FFT to be able to reconstruct phases from FFT data  (smenc -O0)
* use different thresholding scheme for encoder, detecting more partials
* document API with doxygen
* move Encoder and other classes to libspectmorph
* added python binding, capable of reading SpectMorph model files
* added automated tests
* added manual pages for smenc, smplay, smvisualize and smstrip
* added overview document in docs directory

## SpectMorph 0.0.1

* initial public release with three programs
  - smenc       - builds model of a sample
  - smplay      - resynthesizes sample from model
  - smvisualize - visualizes model
