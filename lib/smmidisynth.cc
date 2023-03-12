// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmidisynth.hh"
#include "smmorphoutputmodule.hh"
#include "smdebug.hh"

#include <mutex>
#include <cinttypes>

#include <assert.h>

using namespace SpectMorph;

using std::min;
using std::max;

using std::string;

#define MIDI_DEBUG(...) Debug::debug ("midi", __VA_ARGS__)

#define SM_MIDI_CTL_SUSTAIN       0x40
#define SM_MIDI_CTL_ALL_NOTES_OFF 0x7b

/* control (jack) using General Purpose Controller 1..4 */
#define SM_MIDI_CTL_CONTROL_1     16
#define SM_MIDI_CTL_CONTROL_2     17
#define SM_MIDI_CTL_CONTROL_3     18
#define SM_MIDI_CTL_CONTROL_4     19

MidiSynth::MidiSynth (double mix_freq, size_t n_voices) :
  morph_plan_synth (mix_freq, n_voices),
  m_inst_edit_synth (mix_freq),
  m_mix_freq (mix_freq),
  pedal_down (false),
  audio_time_stamp (0),
  mono_enabled (false),
  portamento_note_id (0),
  next_note_id (1)
{
  voices.clear();
  voices.resize (n_voices);
  active_voices.reserve (n_voices);

  for (size_t i = 0; i < n_voices; i++)
    {
      voices[i].mp_voice = morph_plan_synth.voice (i);
      idle_voices.push_back (&voices[i]);
    }
}

MidiSynth::Voice *
MidiSynth::alloc_voice()
{
  if (idle_voices.empty()) // out of voices?
    return NULL;

  Voice *voice = idle_voices.back();
  assert (voice->state == Voice::STATE_IDLE);   // every item in idle_voices should be idle

  voice->note_id = next_note_id++;

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
          TerminatedVoice tv {
            .key = voice->midi_note,
            .channel = voice->channel,
            .clap_id = voice->clap_id
          };
          MIDI_DEBUG ("terminated voice, clap_id=%d\n", tv.clap_id);
          if (m_process_callbacks)
            m_process_callbacks->terminated_voice (tv);
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
MidiSynth::process_note_on (const TimeInfo& time_info, int channel, int midi_note, int midi_velocity, int clap_id)
{
  // prevent crash without output: ignore note on in this case
  if (!morph_plan_synth.have_output())
    return;

  const MorphOutputModule *output = voices[0].mp_voice->output();
  set_mono_enabled (output->portamento());
  portamento_glide = output->portamento_glide();

  Voice *voice = alloc_voice();
  if (voice)
    {
      voice->freq              = freq_from_note (midi_note);
      voice->pitch_bend_freq   = voice->freq;
      voice->pitch_bend_factor = 0;
      voice->pitch_bend_steps  = 0;
      voice->state             = Voice::STATE_ON;
      voice->midi_note         = midi_note;
      voice->gain              = velocity_to_gain (midi_velocity / 127., output->velocity_sensitivity());
      voice->channel           = channel;
      voice->clap_id           = clap_id;

      voice->modulation.fill (0);

      if (!mono_enabled)
        {
          MorphOutputModule *output = voice->mp_voice->output();

          voice->mono_type = Voice::MonoType::POLY;

          output->retrigger (time_info, 0 /* channel */, voice->freq, midi_velocity);
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

                  mono_voice->freq              = voice->freq;
                  mono_voice->pitch_bend_freq   = voice->pitch_bend_freq;
                  mono_voice->pitch_bend_factor = voice->pitch_bend_factor;
                  mono_voice->pitch_bend_steps  = voice->pitch_bend_steps;
                  mono_voice->state             = voice->state;
                  mono_voice->midi_note         = voice->midi_note;
                  mono_voice->gain              = voice->gain;
                  mono_voice->channel           = voice->channel;


                  mono_voice->mono_type = Voice::MonoType::MONO;

                  output->retrigger (time_info, 0 /* channel */, voice->freq, midi_velocity);
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
  int shadow_midi_note_id = 0;
  for (auto svoice : active_voices)
    {
      if (svoice->state == Voice::STATE_ON && svoice->mono_type == Voice::MonoType::SHADOW)
        {
          /* priorization: new shadow voices are more important than old */
          if (svoice->note_id > shadow_midi_note_id)
            {
              shadow_midi_note = svoice->midi_note;
              shadow_midi_note_id = svoice->note_id;
            }
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

              MorphOutputModule *output_module = mvoice->mp_voice->output();
              output_module->release();
            }
          else if (shadow_midi_note_id != portamento_note_id)
            {
              portamento_note_id = shadow_midi_note_id;

              start_pitch_bend (mvoice, freq_from_note (shadow_midi_note), portamento_glide);
            }
        }
    }
  return found_mono_voice;
}

void
MidiSynth::start_pitch_bend (Voice *voice, double dest_freq, double time_ms)
{
  // require at least one step
  voice->pitch_bend_steps = max (sm_round_positive (time_ms / 1000.0 * m_mix_freq), 1);

  // "steps" multiplications with factor will produce voice->pitch_bend_freq == dest_freq
  voice->pitch_bend_factor = exp (log (dest_freq / voice->pitch_bend_freq) / voice->pitch_bend_steps);
}

void
MidiSynth::process_note_off (int midi_note)
{
  if (mono_enabled)
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

              MorphOutputModule *output_module = voice->mp_voice->output();
              output_module->release();
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

                  MorphOutputModule *output_module = voice->mp_voice->output();
                  output_module->release();
                }
            }
        }
    }
  if (controller == SM_MIDI_CTL_ALL_NOTES_OFF)
    {
      /* release sustain pedal, otherwise note off events will have no effect */
      process_midi_controller (SM_MIDI_CTL_SUSTAIN, 0);

      /* check which notes are active */
      int notes[128] = { 0, };
      for (auto voice : active_voices)
        {
          if (voice->state == Voice::STATE_ON)
            notes[voice->midi_note] = 1;
        }

      /* note off for all active voices */
      for (int note = 0; note < 128; note++)
        {
          if (notes[note])
            process_note_off (note);
        }
    }
  if (m_control_by_cc)
    {
      const float value_f = sm_bound (-1.0, (value / 127.) * 2 - 1, 1.0);
      switch (controller)
        {
          case SM_MIDI_CTL_CONTROL_1: control[0] = value_f;
                                      break;
          case SM_MIDI_CTL_CONTROL_2: control[1] = value_f;
                                      break;
          case SM_MIDI_CTL_CONTROL_3: control[2] = value_f;
                                      break;
          case SM_MIDI_CTL_CONTROL_4: control[3] = value_f;
                                      break;
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
          const double glide_ms = 20.0; /* 20ms smoothing (avoid frequency jumps) */

          start_pitch_bend (voice, voice->freq * pow (2, semi_tones / 12), glide_ms);
        }
    }
}

void
MidiSynth::add_note_on_event (size_t offset, int clap_id, int channel, int key, double velocity)
{
  MidiEvent event;
  event.offset = offset;
  event.key = key;
  event.clap_id = clap_id;
  event.xchannel = channel;
  event.velocity = velocity;
  midi_events.push_back (event);
}

void
MidiSynth::add_midi_event (size_t offset, const unsigned char *midi_data)
{
  if (inst_edit) // inst edit mode? -> delegate
    {
      m_inst_edit_synth.handle_midi_event (midi_data, 0);
      return;
    }
  unsigned char status = midi_data[0] & 0xf0;

  if (status == 0x80 || status == 0x90 || status == 0xb0 || status == 0xe0) // we don't support anything else
    {
      MIDI_DEBUG ("%" PRIu64 " | raw event: status %02x, %02x, %02x\n", audio_time_stamp + offset, status, midi_data[1], midi_data[2]);
      MidiEvent event;
      event.offset = offset;
      event.midi_data[0] = midi_data[0];
      event.midi_data[1] = midi_data[1];
      event.midi_data[2] = midi_data[2];
      midi_events.push_back (event);
    }
  else
    {
      MIDI_DEBUG ("%" PRIu64 " | unhandled event: status %02x, %02x, %02x\n", audio_time_stamp + offset, status, midi_data[1], midi_data[2]);
    }
}

void
MidiSynth::process_audio (const TimeInfo& time_info, float *output, size_t n_values)
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

  for (Voice *voice : active_voices)
    {
      voice->mp_voice->set_control_input (0, std::clamp (control[0] + voice->modulation[0], -1.f, 1.f));
      voice->mp_voice->set_control_input (1, std::clamp (control[1] + voice->modulation[1], -1.f, 1.f));
      voice->mp_voice->set_control_input (2, std::clamp (control[2] + voice->modulation[2], -1.f, 1.f));
      voice->mp_voice->set_control_input (3, std::clamp (control[3] + voice->modulation[3], -1.f, 1.f));

      const float gain = voice->gain * m_gain;
      const float *freq_in = nullptr;
      float frequencies[n_values];
      if (fabs (voice->pitch_bend_freq - voice->freq) > 1e-3 || voice->pitch_bend_steps > 0)
        {
          for (unsigned int i = 0; i < n_values; i++)
            {
              frequencies[i] = voice->pitch_bend_freq;
              if (voice->pitch_bend_steps > 0)
                {
                  voice->pitch_bend_freq *= voice->pitch_bend_factor;
                  voice->pitch_bend_steps--;
                }
            }
          freq_in = frequencies;
        }
      if (voice->mono_type == Voice::MonoType::SHADOW)
        {
          /* skip: shadow voices are not rendered */
        }
      else if (voice->state == Voice::STATE_ON)
        {
          MorphOutputModule *output_module = voice->mp_voice->output();

          output_module->process (time_info, n_values, values, 1, freq_in);
          for (size_t i = 0; i < n_values; i++)
            output[i] += samples[i] * gain;
        }
      else if (voice->state == Voice::STATE_RELEASE)
        {
          MorphOutputModule *output_module = voice->mp_voice->output();

          output_module->process (time_info, n_values, values, 1, freq_in);
          for (size_t i = 0; i < n_values; i++)
            output[i] += samples[i] * gain;

          if (output_module->done())
            {
              /* envelope reached zero -> voice can be reused later */
              voice->state = Voice::STATE_IDLE;
              voice->pedal = false;

              need_free = true; // need to recompute active_voices and idle_voices vectors
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
MidiSynth::process (float *output, size_t n_values, ProcessCallbacks *process_callbacks)
{
  if (inst_edit) // inst edit mode? -> delegate
    {
      m_inst_edit_synth.process (output, n_values, m_notify_buffer);
      return;
    }

  assert (m_process_callbacks == nullptr);
  m_process_callbacks = process_callbacks;

  uint32_t offset = 0;

  TimeInfo time_info;
  time_info.time_ms = audio_time_stamp / m_mix_freq * 1000;
  time_info.ppq_pos = m_ppq_pos;
  morph_plan_synth.update_shared_state (time_info);

  auto offset_cmp = [] (const MidiEvent& a, const MidiEvent& b) { return a.offset < b.offset; };
  if (!std::is_sorted (midi_events.begin(), midi_events.end(), offset_cmp))
    {
      /* Hosts should provide midi events sorted by offset. But if the events
       * are not sorted by offset, we do it here to avoid problems in the event
       * handling code below.
       */
      MIDI_DEBUG ("** got midi events not sorted by offset (this should not happen) **\n");
      std::stable_sort (midi_events.begin(), midi_events.end(), offset_cmp);
    }

  for (const auto& midi_event : midi_events)
    {
      // ensure that new offset from midi event is not larger than n_values
      uint32_t new_offset = min <uint32_t> (midi_event.offset, n_values);

      time_info.time_ms = audio_time_stamp / m_mix_freq * 1000;
      time_info.ppq_pos = m_ppq_pos;

      // process any audio that is before the event
      process_audio (time_info, output + offset, new_offset - offset);
      offset = new_offset;

      if (midi_event.key != -1)
        {
          process_note_on (time_info, midi_event.xchannel, midi_event.key, midi_event.velocity * 127, midi_event.clap_id);
        }
      else if (midi_event.is_pitch_bend())
        {
          const MorphOutputModule *output = voices[0].mp_voice->output();
          const unsigned int lsb = midi_event.midi_data[1];
          const unsigned int msb = midi_event.midi_data[2];
          const unsigned int value = lsb + msb * 128;
          const float semi_tones = (value * (1./0x2000) - 1.0) * output->pitch_bend_range();
          MIDI_DEBUG ("%" PRIu64 " | pitch bend event %d => %.2f semi tones\n", audio_time_stamp, value, semi_tones);
          process_pitch_bend (midi_event.channel(), semi_tones);
        }
      else if (midi_event.is_note_on())
        {
          const int midi_note     = midi_event.midi_data[1];
          const int midi_velocity = midi_event.midi_data[2];

          MIDI_DEBUG ("%" PRIu64 " | note on event, note %d, velocity %d\n", audio_time_stamp, midi_note, midi_velocity);
          process_note_on (time_info, midi_event.channel(), midi_note, midi_velocity, /* note_id */ -1);
        }
      else if (midi_event.is_note_off())
        {
          const int midi_note     = midi_event.midi_data[1];

          MIDI_DEBUG ("%" PRIu64 " | note off event, note %d\n", audio_time_stamp, midi_note);
          process_note_off (midi_note);
        }
      else if (midi_event.is_controller())
        {
          MIDI_DEBUG ("%" PRIu64 " | controller event, %d %d\n", audio_time_stamp, midi_event.midi_data[1], midi_event.midi_data[2]);
          process_midi_controller (midi_event.midi_data[1], midi_event.midi_data[2]);
        }
    }
  time_info.time_ms = audio_time_stamp / m_mix_freq * 1000;
  time_info.ppq_pos = m_ppq_pos;

  // process frames after last event
  process_audio (time_info, output + offset, n_values - offset);

  midi_events.clear();

  m_ppq_pos += n_values * m_tempo / (60. * m_mix_freq);
  m_process_callbacks = nullptr;

  notify_active_voice_status();
}

void
MidiSynth::set_control_input (int i, float value)
{
  assert (i >= 0 && i < MorphPlan::N_CONTROL_INPUTS && !m_control_by_cc);

  control[i] = value;
}

void
MidiSynth::set_modulation (int i, float value)
{
  assert (i >= 0 && i < MorphPlan::N_CONTROL_INPUTS && !m_control_by_cc);

  for (Voice *voice : active_voices)
    voice->modulation[i] = value;
}

void
MidiSynth::set_modulation_clap_id (int i, float value, int clap_id)
{
  assert (i >= 0 && i < MorphPlan::N_CONTROL_INPUTS && !m_control_by_cc);

  for (Voice *voice : active_voices)
    if (voice->clap_id == clap_id)
      voice->modulation[i] = value;
}

void
MidiSynth::set_modulation_key (int i, float value, int key, int channel)
{
  assert (i >= 0 && i < MorphPlan::N_CONTROL_INPUTS && !m_control_by_cc);

  /* FIXME: not tested */
  for (Voice *voice : active_voices)
    if (voice->midi_note == key && voice->channel == channel)
      voice->modulation[i] = value;
}

void
MidiSynth::set_tempo (double tempo)
{
  m_tempo = tempo;
}

void
MidiSynth::set_ppq_pos (double ppq_pos)
{
  m_ppq_pos = ppq_pos;
}

void
MidiSynth::set_mono_enabled (bool new_value)
{
  if (mono_enabled != new_value)
    {
      mono_enabled = new_value;

      /* remove all active voices */
      kill_all_active_voices();
    }
}

void
MidiSynth::kill_all_active_voices()
{
  bool need_free = false;

  for (auto voice : active_voices)
    {
      if (voice->state != Voice::STATE_IDLE)
        {
          voice->state = Voice::STATE_IDLE;
          voice->pedal = false;

          need_free = true;
        }
    }
  if (need_free)
    free_unused_voices();
}

MorphPlanSynth::UpdateP
MidiSynth::prepare_update (MorphPlanPtr plan)
{
  return morph_plan_synth.prepare_update (plan);
}

void
MidiSynth::apply_update (MorphPlanSynth::UpdateP update)
{
  morph_plan_synth.apply_update (update);
}

void
MidiSynth::set_gain (double gain)
{
  m_gain = gain;
}

void
MidiSynth::set_inst_edit (bool iedit)
{
  if (inst_edit != iedit)
    {
      inst_edit = iedit;

      if (inst_edit)
        kill_all_active_voices();
    }
}

InstEditSynth *
MidiSynth::inst_edit_synth()
{
  return &m_inst_edit_synth;
}

double
MidiSynth::mix_freq() const
{
  return m_mix_freq;
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

void
MidiSynth::set_control_by_cc (bool control_by_cc)
{
  m_control_by_cc = control_by_cc;
}

void
MidiSynth::notify_active_voice_status()
{
  if (m_notify_buffer.start_write()) // update notify buffer if GUI has fetched events
    {
      for (auto voice : active_voices)
        voice->mp_voice->fill_notify_buffer (m_notify_buffer);

      m_notify_buffer.write_int (ACTIVE_VOICE_STATUS_EVENT);

      uintptr_t voice_seq[active_voices.size()];
      for (size_t i = 0; i < active_voices.size(); i++)
        voice_seq[i] = (uintptr_t) active_voices[i]->mp_voice;

      m_notify_buffer.write_seq (voice_seq, active_voices.size());

      for (int i = 0; i < MorphPlan::N_CONTROL_INPUTS; i++)
        {
          float control_input_seq[active_voices.size()];

          for (size_t v = 0; v < active_voices.size(); v++)
            control_input_seq[v] = std::clamp (control[i] + active_voices[v]->modulation[i], -1.f, 1.f);

          m_notify_buffer.write_seq (control_input_seq, active_voices.size());
        }
      m_notify_buffer.end_write();
    }
}

NotifyBuffer *
MidiSynth::notify_buffer()
{
  return &m_notify_buffer;
}

// ----notify events----

SynthNotifyEvent *
SynthNotifyEvent::create (NotifyBuffer& buffer)
{
  NotifyEventType type = NotifyEventType (buffer.read_int());
  switch (type)
    {
      case INST_EDIT_VOICE_EVENT:     return new InstEditVoiceEvent (buffer);
      case VOICE_OP_VALUES_EVENT:     return new VoiceOpValuesEvent (buffer);
      case ACTIVE_VOICE_STATUS_EVENT: return new ActiveVoiceStatusEvent (buffer);
      default:                        printf ("unsupported SynthNotifyEvent %d\n", type);
    }
  return nullptr;
}
