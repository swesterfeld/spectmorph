// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "sminsteditsynth.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

using std::string;

static LeakDebugger leak_debugger ("SpectMorph::InstEditSynth");

InstEditSynth::InstEditSynth (float mix_freq) :
  mix_freq (mix_freq)
{
  leak_debugger.add (this);

  voices.resize (64);
}

InstEditSynth::~InstEditSynth()
{
  leak_debugger.del (this);
}

void
InstEditSynth::load_smset (const string& smset, bool enable_original_samples)
{
  wav_set.load (smset);
  for (auto& voice : voices)
    {
      voice.decoder.reset (new LiveDecoder (&wav_set));
      voice.decoder->enable_original_samples (enable_original_samples);
    }
}

static double
note_to_freq (int note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

void
InstEditSynth::handle_midi_event (const unsigned char *midi_data)
{
  const unsigned char status = (midi_data[0] & 0xf0);
  /* note on: */
  if (status == 0x90 && midi_data[2] != 0) /* note on with velocity 0 => note off */
    {
      for (auto& voice : voices)
        {
          if (voice.decoder && voice.state == State::IDLE)
            {
              voice.decoder->retrigger (0, note_to_freq (midi_data[1]), 127, 48000);
              voice.decoder_factor = 1;
              voice.state = State::ON;
              voice.note = midi_data[1];
              return; /* found free voice */
            }
        }
    }

  /* note off */
  if (status == 0x80 || (status == 0x90 && midi_data[2] == 0))
    {
      for (auto& voice : voices)
        {
          if (voice.state == State::ON && voice.note == midi_data[1])
            voice.state = State::RELEASE;
        }
    }
}

void
InstEditSynth::process (float *output, size_t n_values)
{
  zero_float_block (n_values, output);
  for (auto& voice : voices)
    {
      if (voice.decoder && voice.state != State::IDLE)
        {
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
        }
    }
}
