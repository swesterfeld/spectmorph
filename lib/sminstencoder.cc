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

static size_t
make_odd (size_t n)
{
  if (n & 1)
    return n;
  return n - 1;
}

void
InstEncoder::setup_params (const WavData& wav_data, int midi_note)
{
  const double mix_freq = wav_data.mix_freq();
  const int    zeropad  = 4;

  enc_params.mix_freq = mix_freq;
  enc_params.zeropad  = zeropad;
  enc_params.fundamental_freq = freq_from_note (midi_note);

  // --- frame size & step ---
  double min_frame_periods, min_frame_size;
  if (!enc_params.get_param ("min-frame-periods", min_frame_periods))
    min_frame_periods = 4;  // default: at least 4 periods of the fundamental per frame
  if (!enc_params.get_param ("min-frame-size", min_frame_size))
    min_frame_size = 40;    // default: at least 40ms frames

  enc_params.frame_size_ms = min_frame_size;
  enc_params.frame_size_ms = max<float> (enc_params.frame_size_ms, 1000 / enc_params.fundamental_freq * min_frame_periods);
  enc_params.frame_step_ms = enc_params.frame_size_ms / 4.0;

  // --- convert block sizes in ms to sample counts ----
  const size_t  frame_size = make_odd (mix_freq * 0.001 * enc_params.frame_size_ms);
  const size_t  frame_step = mix_freq * 0.001 * enc_params.frame_step_ms;

  /* compute block size from frame size (smallest 2^k value >= frame_size) */
  uint64 block_size = 1;
  while (block_size < frame_size)
    block_size *= 2;

  enc_params.frame_step = frame_step;
  enc_params.frame_size = frame_size;
  enc_params.block_size = block_size;

  enc_params.enable_phases = false; // save some space

  // --- setup window ---
  window.resize (block_size);
  for (uint i = 0; i < window.size(); i++)
    {
      if (i < frame_size)
        window[i] = window_cos (2.0 * i / (frame_size - 1) - 1.0);
      else
        window[i] = 0;
    }
}

Audio *
InstEncoder::encode (const WavData& wav_data, int midi_note, Instrument::EncoderConfig& cfg)
{
  for (auto entry : cfg.entries)
    {
      if (!enc_params.add_config_entry (entry.param, entry.value))
        {
          fprintf (stderr, "InstEncoder: encoder config entry %s is not supported\n", entry.param.c_str());
        }
    }
  setup_params (wav_data, midi_note);

  Encoder encoder (enc_params);

  encoder.encode (wav_data, /* channel */ 0, window, /* opt */ 1, /* attack */ true, /* sines */ true);

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
