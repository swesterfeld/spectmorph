// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SYNTH_INTERFACE_HH
#define SPECTMORPH_SYNTH_INTERFACE_HH

#include "smmidisynth.hh"
namespace SpectMorph
{

class ControlEventVector
{
  std::vector<std::unique_ptr<SynthControlEvent>> events;
  bool clear = false;
public:
  void
  take (SynthControlEvent *ev)
  {
    // we'd rather run destructors in non-rt part of the code
    if (clear)
      {
        events.clear();
        clear = false;
      }

    events.emplace_back (ev);
  }
  void
  run_rt (MidiSynth *midi_synth)
  {
    if (!clear)
      {
        for (const auto& ev : events)
          ev->run_rt (midi_synth);

        clear = true;
      }
  }
};

class SynthInterface
{
public:
  virtual
  ~SynthInterface()
  {
  }
  void
  synth_inst_edit_update (bool active, const std::string& filename, bool original_samples)
  {
    synth_take_control_event (new InstEditUpdate (active, filename, original_samples));
  }
  void
  synth_inst_edit_note (int note, bool on)
  {
    synth_take_control_event (new InstEditNote (note, on));
  }
  virtual void synth_take_control_event (SynthControlEvent *event) = 0;
  virtual std::vector<std::string> notify_take_events() = 0;

  Signal<SynthNotifyEvent *> signal_notify_event;
};

}

#endif
