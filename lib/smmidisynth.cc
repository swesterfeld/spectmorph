// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmidisynth.hh"
#include "smmorphoutputmodule.hh"

#include <assert.h>

using namespace SpectMorph;

using std::min;

using std::string;

#define DEBUG        0
#define MONO_ENABLED 0

static FILE *debug_file = NULL;
QMutex       debug_mutex;

static void
debug (const char *fmt, ...)
{
  if (DEBUG)
    {
      QMutexLocker locker (&debug_mutex);

      if (!debug_file)
        debug_file = fopen ("/tmp/smmidi.log", "w");

      va_list ap;

      va_start (ap, fmt);
      fprintf (debug_file, "%s", string_vprintf (fmt, ap).c_str());
      va_end (ap);
      fflush (debug_file);
    }
}

#define SM_MIDI_CTL_SUSTAIN 0x40

MidiSynth::MidiSynth (double mix_freq, size_t n_voices) :
  morph_plan_synth (mix_freq),
  mix_freq (mix_freq),
  pedal_down (false),
  audio_time_stamp (0),
  control {0, 0}
{
  voices.clear();
  voices.resize (n_voices);
  active_voices.reserve (n_voices);

  for (auto& v : voices)
    {
      v.mp_voice = morph_plan_synth.add_voice();
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
MidiSynth::process_note_on (int channel, int midi_note, int midi_velocity)
{
  // prevent crash without output: ignore note on in this case
  if (!morph_plan_synth.have_output())
    return;

  Voice *voice = alloc_voice();
  if (voice)
    {
      voice->freq            = freq_from_note (midi_note);
      voice->pitch_bend      = 0;
      voice->state           = Voice::STATE_ON;
      voice->midi_note       = midi_note;
      voice->velocity        = midi_velocity / 127.;
      voice->channel         = channel;

      if (!MONO_ENABLED)
        {
          MorphOutputModule *output = voice->mp_voice->output();

          voice->mono_type = Voice::MonoType::POLY;

          output->retrigger (0 /* channel */, voice->freq, midi_velocity);
        }
      else
        {
          voice->mono_type = Voice::MonoType::SHADOW;

          if (!update_mono_voice())
            {
              Voice *mono_voice = alloc_voice();

              if (mono_voice)
                {
                  MorphOutputModule *output = mono_voice->mp_voice->output();

                  mono_voice->freq       = voice->freq;
                  mono_voice->pitch_bend = voice->pitch_bend;
                  mono_voice->state      = voice->state;
                  mono_voice->midi_note  = voice->midi_note;
                  mono_voice->velocity   = voice->velocity;
                  mono_voice->channel    = voice->channel;

                  mono_voice->mono_type = Voice::MonoType::MONO;

                  output->retrigger (0 /* channel */, voice->freq, midi_velocity);
                }
            }
        }
    }
}

bool
MidiSynth::update_mono_voice()
{
  bool found_mono_voice = false;

  /* find active shadow voice */
  int shadow_midi_note = -1;
  for (auto svoice : active_voices)
    {
      if (svoice->state == Voice::STATE_ON && svoice->mono_type == Voice::MonoType::SHADOW)
        {
          /* FIXME: priorization (new shadow voices are more important than old */
          shadow_midi_note = svoice->midi_note;
        }
    }
  /* find main voice */
  for (auto mvoice : active_voices)
    {
      if (mvoice->state == Voice::STATE_ON && mvoice->mono_type == Voice::MonoType::MONO)
        {
          found_mono_voice = true;

          if (shadow_midi_note == -1) /* no more shadow notes point to this note */
            {
              /* pedal not supported in mono mode */
              mvoice->state = Voice::STATE_RELEASE;
              mvoice->env = 1.0;
            }
          else
            {
              mvoice->pitch_bend = shadow_midi_note - mvoice->midi_note;
            }
        }
    }
  return found_mono_voice;
}

void
MidiSynth::process_note_off (int midi_note)
{
  if (MONO_ENABLED)
    {
      bool need_free = false;

      for (auto voice : active_voices)
        {
          if (voice->state == Voice::STATE_ON && voice->midi_note == midi_note && voice->mono_type == Voice::MonoType::SHADOW)
            {
              voice->state = Voice::STATE_IDLE;
              voice->pedal = false; /* pedal not supported in mono mode */

              need_free = true;
            }
        }
      if (need_free)
        free_unused_voices();

      update_mono_voice();
      return;
    }

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
MidiSynth::process_pitch_bend (int channel, double semi_tones)
{
  for (auto voice : active_voices)
    {
      if (voice->state == Voice::STATE_ON && voice->channel == channel)
        {
          voice->pitch_bend = semi_tones;
        }
    }
}

void
MidiSynth::add_midi_event (size_t offset, const unsigned char *midi_data)
{
  unsigned char status = midi_data[0] & 0xf0;

  if (status == 0x80 || status == 0x90 || status == 0xb0 || status == 0xe0) // we don't support anything else
    {
      debug ("%zd | raw event: status %02x, %02x, %02x\n", audio_time_stamp + offset, status, midi_data[1], midi_data[2]);
      MidiEvent event;
      event.offset = offset;
      event.midi_data[0] = midi_data[0];
      event.midi_data[1] = midi_data[1];
      event.midi_data[2] = midi_data[2];
      midi_events.push_back (event);
    }
  else
    {
      debug ("%zd | unhandled event: status %02x, %02x, %02x\n", audio_time_stamp + offset, status, midi_data[1], midi_data[2]);
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

  // prevent crash without output: just return zeros and don't do anything else
  if (!morph_plan_synth.have_output())
    return;

  for (size_t voice_pos = 0; voice_pos < active_voices.size(); voice_pos++)
    {
      Voice *voice = active_voices[voice_pos];

      voice->mp_voice->set_control_input (0, control[0]);
      voice->mp_voice->set_control_input (1, control[1]);

      const float *freq_in = nullptr;
      float frequencies[n_values];
      if (fabs (voice->pitch_bend) > 1e-3)
        {
          double freq = voice->freq * pow (2, voice->pitch_bend / 12);
          for (unsigned int i = 0; i < n_values; i++)
            frequencies[i] = freq;
          freq_in = frequencies;
        }
      if (voice->mono_type == Voice::MonoType::SHADOW)
        {
          /* skip: shadow voices are not rendered */
        }
      else if (voice->state == Voice::STATE_ON)
        {
          MorphOutputModule *output_module = voice->mp_voice->output();

          output_module->process (n_values, values, 1, freq_in);
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

          output_module->process (envelope_len, values, 1, freq_in);
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

  audio_time_stamp += n_values;
}

void
MidiSynth::process (float *output, size_t n_values)
{
  uint32_t offset = 0;

  for (const auto& midi_event : midi_events)
    {
      // ensure that new offset from midi event is not larger than n_values
      uint32_t new_offset = min <uint32_t> (midi_event.offset, n_values);

      // process any audio that is before the event
      process_audio (output + offset, new_offset - offset);
      offset = new_offset;

      if (midi_event.is_pitch_bend())
        {
          const unsigned int lsb = midi_event.midi_data[1];
          const unsigned int msb = midi_event.midi_data[2];
          const unsigned int value = lsb + msb * 128;
          const float semi_tones = (value * (1./0x2000) - 1.0) * 48;
          debug ("%zd | pitch bend event %d => %.2f semi tones\n", audio_time_stamp, value, semi_tones);
          process_pitch_bend (midi_event.channel(), semi_tones);
        }
      if (midi_event.is_note_on())
        {
          const int midi_note     = midi_event.midi_data[1];
          const int midi_velocity = midi_event.midi_data[2];

          debug ("%zd | note on event, note %d, velocity %d\n", audio_time_stamp, midi_note, midi_velocity);
          process_note_on (midi_event.channel(), midi_note, midi_velocity);
        }
      else if (midi_event.is_note_off())
        {
          const int midi_note     = midi_event.midi_data[1];

          debug ("%zd | note off event, note %d\n", audio_time_stamp, midi_note);
          process_note_off (midi_note);
        }
      else if (midi_event.is_controller())
        {
          debug ("%zd | controller event, %d %d\n", audio_time_stamp, midi_event.midi_data[1], midi_event.midi_data[2]);
          process_midi_controller (midi_event.midi_data[1], midi_event.midi_data[2]);
        }
    }
  // process frames after last event
  process_audio (output + offset, n_values - offset);

  midi_events.clear();

  morph_plan_synth.update_shared_state (n_values / mix_freq * 1000);
}

void
MidiSynth::set_control_input (int i, float value)
{
  assert (i >= 0 && i < 2);

  control[i] = value;
}

void
MidiSynth::update_plan (MorphPlanPtr new_plan)
{
  morph_plan_synth.update_plan (new_plan);
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

bool
MidiSynth::MidiEvent::is_pitch_bend() const
{
  return (midi_data[0] & 0xf0) == 0xe0;
}

int
MidiSynth::MidiEvent::channel() const
{
  return midi_data[0] & 0xf;
}
