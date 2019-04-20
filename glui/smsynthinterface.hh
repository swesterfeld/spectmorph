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
  template<class DATA>
  void
  send_control_event (const std::function<void(MidiSynth *)>& func, DATA *data = nullptr)
  {
    synth_take_control_event (new InstFunc (func, [data]() { delete data;}));
  }
  void
  send_control_event (const std::function<void(MidiSynth *)>& func)
  {
    synth_take_control_event (new InstFunc (func, []() {}));
  }
  void
  synth_inst_edit_update (bool active, WavSet *take_wav_set, bool original_samples)
  {
    /* ownership of take_wav_set is transferred to the event */
    struct EventData
    {
      std::unique_ptr<WavSet> wav_set;
    } *event_data = new EventData;

    event_data->wav_set.reset (take_wav_set);

    send_control_event (
      [=] (MidiSynth *midi_synth)
        {
          midi_synth->set_inst_edit (active);

          if (active)
            midi_synth->inst_edit_synth()->take_wav_set (event_data->wav_set.release(), original_samples);
        },
      event_data);
  }
  void
  synth_inst_edit_note (int note, bool on)
  {
    send_control_event (
      [=] (MidiSynth *midi_synth)
        {
          unsigned char event[3];

          event[0] = on ? 0x90 : 0x80;
          event[1] = note;
          event[2] = on ? 100 : 0;

          midi_synth->add_midi_event (0, event);
        });
  }
  virtual void synth_take_control_event (SynthControlEvent *event) = 0;
  virtual std::vector<std::string> notify_take_events() = 0;

  Signal<SynthNotifyEvent *> signal_notify_event;
};

}

#endif
