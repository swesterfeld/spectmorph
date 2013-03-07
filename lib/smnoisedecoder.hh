// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_NOISE_DECODER_HH
#define SPECTMORPH_NOISE_DECODER_HH

#include "smrandom.hh"
#include "smnoisebandpartition.hh"
#include "smaudio.hh"

namespace SpectMorph
{

/**
 * \brief Decoder for the noise component (stochastic component) of the signal
 */
class NoiseDecoder
{
  double orig_mix_freq;
  double mix_freq;
  size_t block_size;

  float *cos_window;

  Random random_gen;
  NoiseBandPartition *noise_band_partition;

  void apply_window (float *spectrum, float *fft_buffer);

public:
  NoiseDecoder (double orig_mix_freq,
                double mix_freq,
                size_t block_size);
  ~NoiseDecoder();

  enum OutputMode { REPLACE, ADD, FFT_SPECTRUM, DEBUG_UNWINDOWED, DEBUG_NO_OUTPUT };

  void set_seed (int seed);
  void process (const AudioBlock& audio_block,
                float *samples,
                OutputMode output_mode = REPLACE);

  static size_t preferred_block_size (double mix_freq);
};

}

#endif
