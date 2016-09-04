// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmidisynth.hh"
#include "smmorphoutputmodule.hh"

#include <assert.h>

using namespace SpectMorph;

using std::min;

#define SM_MIDI_CTL_SUSTAIN 0x40

MidiSynth::MidiSynth (MorphPlanSynth& synth, double mix_freq, size_t n_voices) :
  mix_freq (mix_freq),
  pedal_down (false),
  control {0, 0}
{
  voices.clear();
  voices.resize (n_voices);
  active_voices.reserve (n_voices);

  for (auto& v : voices)
    {
      v.mp_voice = synth.add_voice();
      idle_voices.push_back (&v);
    }
}

MidiSynth::Voice *
MidiSynth::alloc_voice()
{
  if (idle_voices.empty()) // out of voices?
    return NULL;

  Voice *voice = idle_voices.back();
  assert (voice->state == Voice::STATE_IDLE);   // every item in idle_voices should be idle

  // move voice from idle to active list
  idle_voices.pop_back();
  active_voices.push_back (voice);

  return voice;
}

void
MidiSynth::free_unused_voices()
{
  size_t new_voice_count = 0;

  for (size_t i = 0; i < active_voices.size(); i++)
    {
      Voice *voice = active_voices[i];

      if (voice->state == Voice::STATE_IDLE)    // voice used?
        {
          idle_voices.push_back (voice);
        }
      else
        {
          active_voices[new_voice_count++] = voice;
        }
    }
  active_voices.resize (new_voice_count);
}

size_t
MidiSynth::active_voice_count() const
{
  return active_voices.size();
}

float
MidiSynth::freq_from_note (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

void
MidiSynth::process_note_on (int midi_note, int midi_velocity)
{
  Voice *voice = alloc_voice();
  if (voice)
    {
      MorphOutputModule *output = voice->mp_voice->output();
      output->retrigger (0 /* channel */, freq_from_note (midi_note), midi_velocity);

      voice->state     = Voice::STATE_ON;
      voice->midi_note = midi_note;
      voice->velocity  = midi_velocity / 127.;
    }
}

void
MidiSynth::process_note_off (int midi_note)
{
  for (auto voice : active_voices)
    {
      if (voice->state == Voice::STATE_ON && voice->midi_note == midi_note)
        {
          if (pedal_down)
            {
              voice->pedal = true;
            }
          else
            {
              voice->state = Voice::STATE_RELEASE;
              voice->env = 1.0;
            }
        }
    }
}

void
MidiSynth::process_midi_controller (int controller, int value)
{
  if (controller == SM_MIDI_CTL_SUSTAIN)
    {
      pedal_down = value > 0x40;
      if (!pedal_down)
        {
          /* release voices which are sustained due to the pedal */
          for (auto voice : active_voices)
            {
              if (voice->pedal && voice->state == Voice::STATE_ON)
                {
                  voice->state = Voice::STATE_RELEASE;
                  voice->env = 1.0;
                }
            }
        }
    }
}

void
MidiSynth::add_midi_event (size_t offset, unsigned char *midi_data)
{
  unsigned char status = midi_data[0] & 0xf0;

  if (status == 0x80 || status == 0x90 || status == 0xb0) // we don't support anything else
    {
      MidiEvent event;
      event.offset = offset;
      event.midi_data[0] = midi_data[0];
      event.midi_data[1] = midi_data[1];
      event.midi_data[2] = midi_data[2];
      midi_events.push_back (event);
    }
}

void
MidiSynth::process_audio (float *output, size_t n_values)
{
  if (!n_values)    /* this can happen if multiple midi events occur at the same time */
    return;

  bool  need_free = false;
  float samples[n_values];
  float *values[1] = { samples };

  zero_float_block (n_values, output);

  for (size_t voice_pos = 0; voice_pos < active_voices.size(); voice_pos++)
    {
      Voice *voice = active_voices[voice_pos];

      voice->mp_voice->set_control_input (0, control[0]);
      voice->mp_voice->set_control_input (1, control[1]);

      if (voice->state == Voice::STATE_ON)
        {
          MorphOutputModule *output_module = voice->mp_voice->output();

          output_module->process (n_values, values, 1);
          for (size_t i = 0; i < n_values; i++)
            output[i] += samples[i] * voice->velocity;
        }
      else if (voice->state == Voice::STATE_RELEASE)
        {
          const float release_ms = 150; /* FIXME: this should be set by the user */

          double v_decrement = (1000.0 / mix_freq) / release_ms;
          size_t envelope_len = qBound<int> (0, sm_round_positive (voice->env / v_decrement), n_values);

          if (envelope_len < n_values)
            {
              /* envelope reached zero -> voice can be reused later */
              voice->state = Voice::STATE_IDLE;
              voice->pedal = false;

              need_free = true; // need to recompute active_voices and idle_voices vectors
            }
          MorphOutputModule *output_module = voice->mp_voice->output();

          output_module->process (envelope_len, values, 1);
          for (size_t i = 0; i < envelope_len; i++)
            {
              voice->env -= v_decrement;
              output[i] += samples[i] * voice->env * voice->velocity;
            }
        }
      else
        {
          g_assert_not_reached();
        }
    }
  if (need_free)
    free_unused_voices();
}

void
MidiSynth::process_audio_midi (float *output, size_t n_values)
{
  uint32_t offset = 0;

  for (const auto& midi_event : midi_events)
    {
      // ensure that new offset from midi event is not larger than n_values
      uint32_t new_offset = min <uint32_t> (midi_event.offset, n_values);

      // process any audio that is before the event
      process_audio (output + offset, new_offset - offset);
      offset = new_offset;

      if (midi_event.is_note_on())
        {
          const int midi_note     = midi_event.midi_data[1];
          const int midi_velocity = midi_event.midi_data[2];

          process_note_on (midi_note, midi_velocity);
        }
      else if (midi_event.is_note_off())
        {
          const int midi_note     = midi_event.midi_data[1];

          process_note_off (midi_note);
        }
      else if (midi_event.is_controller())
        {
          process_midi_controller (midi_event.midi_data[1], midi_event.midi_data[2]);
        }
    }
  // process frames after last event
  process_audio (output + offset, n_values - offset);

  midi_events.clear();
}

void
MidiSynth::set_control_input (int i, float value)
{
  assert (i >= 0 && i < 2);

  control[i] = value;
}

// midi event classification functions
bool
MidiSynth::MidiEvent::is_note_on() const
{
  if ((midi_data[0] & 0xf0) == 0x90)
    {
      if (midi_data[2] != 0) /* note on with velocity 0 => note off */
        return true;
    }
  return false;
}

bool
MidiSynth::MidiEvent::is_note_off() const
{
  if ((midi_data[0] & 0xf0) == 0x90)
    {
      if (midi_data[2] == 0) /* note on with velocity 0 => note off */
        return true;
    }
  else if ((midi_data[0] & 0xf0) == 0x80)
    {
      return true;
    }
  return false;
}

bool
MidiSynth::MidiEvent::is_controller() const
{
  return (midi_data[0] & 0xf0) == 0xb0;
}
