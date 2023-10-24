// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_SYNTH_INTERFACE_HH
#define SPECTMORPH_SYNTH_INTERFACE_HH

#include "smmidisynth.hh"
#include "smproject.hh"
#include "smmorphplansynth.hh"

namespace SpectMorph
{

class SynthInterface
{
  Project *m_project = nullptr;
public:
  SynthInterface (Project *project)
  {
    m_project = project;
  }
  Project *
  get_project()
  {
    return m_project;
  }
  template<class DATA>
  void
  send_control_event (const std::function<void(Project *)>& func, DATA *data = nullptr)
  {
    m_project->synth_take_control_event (new InstFunc (func, [data]() { delete data;}));
  }
  void
  send_control_event (const std::function<void(Project *)>& func)
  {
    m_project->synth_take_control_event (new InstFunc (func, []() {}));
  }
  void
  synth_inst_edit_update (bool active, WavSet *take_wav_set, WavSet *ref_wav_set)
  {
    /* ownership:
     *  - take_wav_set is owned by the event
     *  - ref_wav_set is _not_ owned and will not be freed
     */
    struct EventData
    {
      InstEditSynth::Decoders decoders;
    } *event_data = new EventData;

    event_data->decoders = m_project->midi_synth()->inst_edit_synth()->create_decoders (take_wav_set, ref_wav_set);

    send_control_event (
      [=] (Project *project)
        {
          project->midi_synth()->set_inst_edit (active);

          if (active)
            project->midi_synth()->inst_edit_synth()->swap_decoders (event_data->decoders);
        },
      event_data);
  }
  void
  synth_inst_edit_gain (float gain)
  {
    send_control_event (
      [=] (Project *project)
        {
          project->midi_synth()->inst_edit_synth()->set_gain (gain);
        });
  }
  void
  synth_inst_edit_midi_to_reference (bool midi_to_reference)
  {
    send_control_event (
      [=] (Project *project)
        {
          project->midi_synth()->inst_edit_synth()->set_midi_to_reference (midi_to_reference);
        });
  }
  void
  synth_inst_edit_note (int note, bool on, unsigned int layer)
  {
    send_control_event (
      [=] (Project *project)
        {
          if (on)
            project->midi_synth()->inst_edit_synth()->process_note_on (/* channel */ 0, note, /* clap id */ -1, layer);
          else
            project->midi_synth()->inst_edit_synth()->process_note_off (/* channel */ 0, note, layer);
        });
  }
  void
  emit_apply_update (MorphPlanSynth::UpdateP update)
  {
    /* ownership of update is transferred to the event */
    struct EventData
    {
      MorphPlanSynth::UpdateP update;
    } *event_data = new EventData;

    event_data->update = update;
    send_control_event (
      [=] (Project *project)
        {
          project->midi_synth()->apply_update (event_data->update);
        },
      event_data);
  }
  void
  emit_update_gain (double gain)
  {
    send_control_event (
      [=] (Project *project)
        {
          project->midi_synth()->set_gain (gain);
        });
  }
  void
  emit_add_rebuild_result (int object_id, WavSet *take_wav_set)
  {
    /* ownership of take_wav_set is transferred to the event */
    struct EventData
    {
      std::unique_ptr<WavSet> wav_set;
    } *event_data = new EventData;

    event_data->wav_set.reset (take_wav_set);

    send_control_event (
      [=] (Project *project)
        {
          // uses swap to assign the new unique ptr and ensure the old one gets freed
          // outside the audio thread
          project->add_rebuild_result (object_id, event_data->wav_set);
        },
        event_data);
  }
  void
  emit_clear_wav_sets()
  {
    struct EventData
    {
      std::vector<std::unique_ptr<WavSet>> wav_sets;
    } *event_data = new EventData;

    /* avoid malloc in audio threads if wav sets are added */
    event_data->wav_sets.reserve (Project::WAV_SETS_RESERVE);

    send_control_event (
      [=] (Project *project)
        {
          // uses swap to ensure that old wav_sets get freed outside the audio thread
          project->clear_wav_sets (event_data->wav_sets);
        },
        event_data);
  }
  void
  generate_notify_events()
  {
    NotifyBuffer *notify_buffer = m_project->notify_buffer();
    if (notify_buffer->start_read())
      {
        while (notify_buffer->remaining())
          {
            SynthNotifyEvent *sn_event = SynthNotifyEvent::create (*notify_buffer);
            signal_notify_event (sn_event);
            delete sn_event;
          }
        notify_buffer->end_read();
      }
    else
      {
        // this allocates memory, but it is OK because we are in the UI thread
        notify_buffer->resize_if_necessary();
      }
  }
  Signal<SynthNotifyEvent *> signal_notify_event;
};

}

#endif
