// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROJECT_HH
#define SPECTMORPH_PROJECT_HH

#include "sminstrument.hh"
#include "smwavset.hh"
#include "smwavsetbuilder.hh"

#include <thread>
#include <mutex>

namespace SpectMorph
{

class MidiSynth;

class SynthControlEvent
{
public:
  virtual void run_rt (MidiSynth *midi_synth) = 0;
  virtual
  ~SynthControlEvent()
  {
  }
};

class ControlEventVector
{
  std::vector<std::unique_ptr<SynthControlEvent>> events;
  bool clear = false;
public:
  void take (SynthControlEvent *ev);
  void run_rt (MidiSynth *midi_synth);
};

class Project
{
  std::shared_ptr<WavSet> wav_set = nullptr;

  std::mutex              m_synth_mutex;
  MidiSynth              *m_midi_synth = nullptr;
  ControlEventVector      m_control_events;
public:
  Instrument instrument;

  void
  rebuild()
  {
    WavSetBuilder *builder = new WavSetBuilder (&instrument, /* keep_samples */ false);
    new std::thread ([this, builder]() {
      /* FIXME: sharing a pointer between threads can crash */
      wav_set = std::shared_ptr<WavSet> (builder->run());
      delete builder;
    });
  }
  std::shared_ptr<WavSet>
  get_wav_set()
  {
    return wav_set;
  }

  void synth_take_control_event (SynthControlEvent *event);

  std::mutex&
  synth_mutex()
  {
    /* the synthesis thread will typically not block on synth_mutex
     * instead, it tries locking it, and if that fails, continues
     *
     * the ui thread will block on the synth_mutex to enqueue events,
     * parameter changes (in form of a new morph plan, volume, ...)
     * and so on
     *
     * if the synthesis thread can obtain a lock, it will then be
     * able to process these events to update its internal state
     * and also send notifications back to the ui
     */
    return m_synth_mutex;
  }
  void try_update_synth();
  void
  change_midi_synth (MidiSynth *midi_synth)
  {
    m_midi_synth = midi_synth;
  }
};

}

#endif
