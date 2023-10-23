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

InstEditSynth::Decoders
InstEditSynth::create_decoders (WavSet *take_wav_set, WavSet *ref_wav_set, bool midi_to_reference)
{
  // this code does not run in audio thread, so it can do the slow setup stuff (alloc memory)
  Decoders decoders;

  decoders.wav_set.reset (take_wav_set);
  decoders.midi_to_reference = midi_to_reference;
  for (unsigned int v = 0; v < voices_per_layer; v++)
    {
      auto layer0_decoder = new LiveDecoder (decoders.wav_set.get(), mix_freq);

      auto layer1_decoder = new LiveDecoder (decoders.wav_set.get(), mix_freq);
      layer1_decoder->enable_original_samples (true);

      auto layer2_decoder = new LiveDecoder (ref_wav_set, mix_freq);

      decoders.decoders.emplace_back (layer0_decoder);
      decoders.decoders.emplace_back (layer1_decoder);
      decoders.decoders.emplace_back (layer2_decoder);
    }
  return decoders;
}

void
InstEditSynth::swap_decoders (Decoders& new_decoders)
{
  // this code does run in audio thread, so it should avoid malloc() / free()
  assert (new_decoders.decoders.size() == voices.size());

  for (size_t vidx = 0; vidx < voices.size(); vidx++)
    voices[vidx].decoder = new_decoders.decoders[vidx].get();

  decoders.midi_to_reference = new_decoders.midi_to_reference;
  decoders.wav_set.swap (new_decoders.wav_set);
  decoders.decoders.swap (new_decoders.decoders);
}

static double
note_to_freq (int note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

void
InstEditSynth::set_gain (float new_gain)
{
  gain = new_gain;
}

void
InstEditSynth::process_note_on (int channel, int note, int clap_id, int layer)
{
  if (layer == -1) /* layer -1: midi events */
    {
      if (decoders.midi_to_reference)
        layer = 2;
      else
        layer = 0;
    }
  for (auto& voice : voices)
    {
      if (voice.decoder && voice.state == State::IDLE && int (voice.layer) == layer)
        {
          voice.decoder->retrigger (0, note_to_freq (note), 127);
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
InstEditSynth::process_note_off (int channel, int note, int layer)
{
  for (auto& voice : voices)
    {
      if (voice.state == State::ON && voice.channel == channel && voice.note == note && (int (voice.layer) == layer || layer == -1))
        voice.state = State::RELEASE;
    }
}

void
InstEditSynth::process (float *output, size_t n_values, RTMemoryArea& rt_memory_area, NotifyBuffer& notify_buffer, MidiSynthCallbacks *process_callbacks)
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

          voice.decoder->process (rt_memory_area, n_values, nullptr, &samples[0]);

          const float release_ms = 150;
          const float decrement = (1000.0 / mix_freq) / release_ms;

          for (size_t i = 0; i < n_values; i++)
            {
              if (voice.state == State::ON)
                {
                  output[i] += samples[i] * gain; /* pass */
                }
              else if (voice.state == State::RELEASE)
                {
                  voice.decoder_factor -= decrement;

                  if (voice.decoder_factor > 0)
                    output[i] += samples[i] * gain * voice.decoder_factor;
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
