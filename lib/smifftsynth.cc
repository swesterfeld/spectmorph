// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smifftsynth.hh"
#include "smmath.hh"
#include "smfft.hh"
#include "smblockutils.hh"
#include <assert.h>
#include <stdio.h>

#include <map>

using namespace SpectMorph;

using std::vector;
using std::map;

static map<size_t, IFFTSynthTable *> table_for_block_size;

namespace SpectMorph {
  vector<float> IFFTSynth::sin_table;
}

IFFTSynth::IFFTSynth (size_t block_size, double mix_freq, WindowType win_type) :
  block_size (block_size),
  mix_freq (mix_freq)
{
  zero_padding = 256;

  table = table_for_block_size[block_size];
  if (!table)
    {
      const int range = 4;

      table = new IFFTSynthTable();

      const size_t win_size = block_size * zero_padding;
      float *win = FFT::new_array_float (win_size);
      float *wspectrum = FFT::new_array_float (win_size);

      std::fill (win, win + win_size, 0);  // most of it should be zero due to zeropadding
      for (size_t i = 0; i < block_size; i++)
        {
          if (i < block_size / 2)
            win[i] = window_blackman_harris_92 (double (block_size / 2 - i) / block_size * 2 - 1.0);
          else
            win[win_size - block_size + i] = window_blackman_harris_92 (double (i - block_size / 2) / block_size * 2 - 1.0);
        }

      FFT::fftar_float (block_size * zero_padding, win, wspectrum, FFT::PLAN_ESTIMATE);

      // compute complete (symmetric) expanded window transform for all frequency fractions
      for (int freq_frac = 0; freq_frac < zero_padding; freq_frac++)
        {
          for (int i = -range; i <= range; i++)
            {
              int pos = i * 256 - freq_frac;
              table->win_trans.push_back (wspectrum[abs (pos * 2)]);
            }
        }
      FFT::free_array_float (win);
      FFT::free_array_float (wspectrum);

      table->win_scale = FFT::new_array_float (block_size); // SSE
      for (size_t i = 0; i < block_size; i++)
        table->win_scale[(i + block_size / 2) % block_size] = window_cos (2.0 * i / block_size - 1.0) / window_blackman_harris_92 (2.0 * i / block_size - 1.0);

      // we only need to do this once per block size (FIXME: not thread safe yet)
      table_for_block_size[block_size] = table;
    }
  if (sin_table.empty())
    {
      // sin() table
      sin_table.resize (SIN_TABLE_SIZE);
      for (size_t i = 0; i < SIN_TABLE_SIZE; i++)
        sin_table[i] = sin (i * 2 * M_PI / SIN_TABLE_SIZE);
    }

  if (win_type == WIN_BLACKMAN_HARRIS_92)
    win_scale = NULL;
  else
    win_scale = table->win_scale;

  fft_in = FFT::new_array_float (block_size);
  fft_out = FFT::new_array_float (block_size);
  freq256_factor = 1 / mix_freq * block_size * zero_padding;
  mag_norm = 0.5 / block_size;
}

IFFTSynth::~IFFTSynth()
{
  FFT::free_array_float (fft_in);
  FFT::free_array_float (fft_out);
}

void
IFFTSynth::get_samples (float      *samples,
                        OutputMode  output_mode)
{
  FFT::fftsr_destructive_float (block_size, fft_in, fft_out);

  if (win_scale)
    Block::mul (block_size, fft_out, win_scale);

  if (output_mode == REPLACE)
    {
      memcpy (samples, &fft_out[block_size / 2], sizeof (float) * block_size / 2);
      memcpy (&samples[block_size / 2], fft_out, sizeof (float) * block_size / 2);
    }
  else if (output_mode == ADD)
    {
      Block::add (block_size / 2, samples, fft_out + block_size / 2);
      Block::add (block_size / 2, samples + block_size / 2, fft_out);
    }
  else
    {
      assert (false);
    }
}

double
IFFTSynth::quantized_freq (double mf_freq)
{
  const int freq256 = sm_round_positive (mf_freq * freq256_factor);
  const double qfreq = freq256 * (1 / 256.0);
  const double mf_qfreq = qfreq / block_size * mix_freq;

  return mf_qfreq;
}
