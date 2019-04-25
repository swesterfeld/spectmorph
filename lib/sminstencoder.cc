// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "sminstencoder.hh"

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

static float
freq_from_note (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

Audio *
InstEncoder::encode (const WavData& wav_data, int midi_note, Instrument::EncoderConfig& cfg, const std::function<bool()>& kill_function)
{
  if (cfg.enabled)
    {
      for (auto entry : cfg.entries)
        {
          if (!enc_params.add_config_entry (entry.param, entry.value))
            {
              fprintf (stderr, "InstEncoder: encoder config entry %s is not supported\n", entry.param.c_str());
            }
        }
    }
  enc_params.setup_params (wav_data, freq_from_note (midi_note));
  enc_params.enable_phases = false; // save some space
  enc_params.set_kill_function (kill_function);

  Encoder encoder (enc_params);

  if (!encoder.encode (wav_data, /* channel */ 0, /* opt */ 1, /* attack */ true, /* sines */ true))
    return nullptr;

  /* strip stuff we don't need (but keep everything that is needed if loop points are changed) */
  vector<EncoderBlock>& audio_blocks = encoder.audio_blocks;

  for (size_t i = 0; i < audio_blocks.size(); i++)
    {
      audio_blocks[i].debug_samples.clear();
      audio_blocks[i].original_fft.clear();
    }
  encoder.original_samples.clear();

  return encoder.save_as_audio();
}
