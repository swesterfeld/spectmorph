// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "sminsteditsynth.hh"
#include "smleakdebugger.hh"
#include "smmidisynth.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::InstEditSynth");

InstEditSynth::InstEditSynth (float mix_freq) :
  mix_freq (mix_freq)
{
  leak_debugger.add (this);

  unsigned int voices_per_layer = 64;
  for (unsigned int v = 0; v < voices_per_layer; v++)
    {
      for (unsigned int layer = 0; layer < n_layers; layer++)
        {
          Voice voice;
          voice.layer = layer;

          voices.push_back (std::move (voice));
        }
    }
}

InstEditSynth::~InstEditSynth()
{
  leak_debugger.del (this);
}

void
InstEditSynth::take_wav_sets (WavSet *new_wav_set, WavSet *new_ref_wav_set)
{
  wav_set.reset (new_wav_set);
  ref_wav_set.reset (new_ref_wav_set);
  for (auto& voice : voices)
    {
      if (voice.layer == 0)
        {
          voice.decoder.reset (new LiveDecoder (wav_set.get()));
        }
      if (voice.layer == 1)
        {
          voice.decoder.reset (new LiveDecoder (wav_set.get()));
          voice.decoder->enable_original_samples (true);
        }
      if (voice.layer == 2)
        {
          voice.decoder.reset (new LiveDecoder (ref_wav_set.get()));
        }
    }
}

static double
note_to_freq (int note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

void
InstEditSynth::process_note_on (int channel, int note, int clap_id, unsigned int layer)
{
  for (auto& voice : voices)
    {
      if (voice.decoder && voice.state == State::IDLE && voice.layer == layer)
        {
          voice.decoder->retrigger (0, note_to_freq (note), 127, mix_freq);
          voice.decoder_factor = 1;
          voice.state = State::ON;
          voice.channel = channel;
          voice.note = note;
          voice.clap_id = clap_id;
          return; /* found free voice */
        }
    }
}

void
InstEditSynth::process_note_off (int channel, int note, unsigned int layer)
{
  for (auto& voice : voices)
    {
      if (voice.state == State::ON && voice.channel == channel && voice.note == note && voice.layer == layer)
        voice.state = State::RELEASE;
    }
}

void
InstEditSynth::process (float *output, size_t n_values, NotifyBuffer& notify_buffer, MidiSynthCallbacks *process_callbacks)
{
  InstEditVoiceEvent::Voice iev[voices.size()];
  int                       iev_len = 0;

  zero_float_block (n_values, output);
  for (auto& voice : voices)
    {
      if (voice.decoder && voice.state != State::IDLE)
        {
          iev[iev_len].note             = voice.note;
          iev[iev_len].layer            = voice.layer;
          iev[iev_len].current_pos      = voice.decoder->current_pos();
          iev[iev_len].fundamental_note = voice.decoder->fundamental_note();
          iev_len++;

          float samples[n_values];

          voice.decoder->process (n_values, nullptr, &samples[0]);

          const float release_ms = 150;
          const float decrement = (1000.0 / mix_freq) / release_ms;

          for (size_t i = 0; i < n_values; i++)
            {
              if (voice.state == State::ON)
                {
                  output[i] += samples[i]; /* pass */
                }
              else if (voice.state == State::RELEASE)
                {
                  voice.decoder_factor -= decrement;

                  if (voice.decoder_factor > 0)
                    output[i] += samples[i] * voice.decoder_factor;
                  else
                    voice.state = State::IDLE;
                }
            }
          if (voice.decoder->done())
            voice.state = State::IDLE;

          if (voice.state == State::IDLE && process_callbacks)
            {
              MidiSynthCallbacks::TerminatedVoice tv {
                .key = voice.note,
                .channel = voice.channel,
                .clap_id = voice.clap_id
              };
              process_callbacks->terminated_voice (tv);
            }
        }
    }

  if (notify_buffer.start_write()) // update notify buffer if GUI has fetched events
    {
      notify_buffer.write_int (INST_EDIT_VOICE_EVENT);
      notify_buffer.write_seq (iev, iev_len);
      notify_buffer.end_write();
    }
}
