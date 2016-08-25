// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmidisynth.hh"
#include "smmorphoutputmodule.hh"

#include <assert.h>

using namespace SpectMorph;

#define SM_MIDI_CTL_SUSTAIN 0x40

MidiSynth::MidiSynth (MorphPlanSynth& synth, double mix_freq, size_t n_voices) :
  mix_freq (mix_freq),
  pedal_down (false)
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
MidiSynth::free_voice (size_t pos)
{
  Voice *voice = active_voices[pos];
  assert (voice->state == Voice::STATE_IDLE);   // voices should be marked idle before freeing

  // replace this voice entry with the last entry of the list
  active_voices[pos] = active_voices.back();

  // move voice from active to idle list
  active_voices.pop_back();
  idle_voices.push_back (voice);
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
MidiSynth::process_audio (float *output, size_t n_values)
{
  zero_float_block (n_values, output);

  float samples[n_values];
  float *values[1] = { samples };

  for (size_t voice_pos = 0; voice_pos < active_voices.size(); voice_pos++)
    {
      Voice *voice = active_voices[voice_pos];

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

              free_voice (voice_pos);
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
}


