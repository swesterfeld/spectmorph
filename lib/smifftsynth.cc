// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smifftsynth.hh"
#include "smmath.hh"
#include "smfft.hh"
#include "smblockutils.hh"
#include "smmain.hh"
#include <assert.h>
#include <stdio.h>

#include <map>
#include <mutex>

using namespace SpectMorph;

using std::vector;
using std::map;

namespace
{

class IFFTSynthGlobal
{
public:
  static IFFTSynthGlobal *
  the()
  {
    static Singleton<IFFTSynthGlobal> singleton;
    return singleton.ptr();
  }
  std::mutex                    mutex;
  map<size_t, IFFTSynthTable *> table_for_block_size;
  bool                          sin_table_initialized = false;

  ~IFFTSynthGlobal()
  {
    for (auto map_it : table_for_block_size)
      delete map_it.second;
  }
};

}

IFFTSynth::IFFTSynth (size_t block_size, double mix_freq, WindowType win_type) :
  block_size (block_size)
{
  IFFTSynthGlobal *global = IFFTSynthGlobal::the();

  std::lock_guard lg (global->mutex);

  zero_padding = 256;

  table = global->table_for_block_size[block_size];
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

      global->table_for_block_size[block_size] = table;
    }
  if (!global->sin_table_initialized)
    {
      // sin() table
      for (size_t i = 0; i < SIN_TABLE_SIZE; i++)
        sin_table[i] = sin (i * 2 * M_PI / SIN_TABLE_SIZE);
      global->sin_table_initialized = true;
    }

  if (win_type == WIN_BLACKMAN_HARRIS_92)
    win_scale = NULL;
  else
    win_scale = table->win_scale;

  fft_in = FFT::new_array_float (block_size);
  fft_out = FFT::new_array_float (block_size);
  fft_plan = FFT::plan_fftsr_destructive_float (block_size); // not RT-safe due to locking

  freq256_factor = 1 / mix_freq * block_size * zero_padding;
  freq256_to_qfreq = mix_freq / 256.0 / block_size;
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
  FFT::execute_fftsr_destructive_float (block_size, fft_in, fft_out, fft_plan);

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

void
IFFTSynth::precompute_tables()
{
  // trigger fftw planning which can be slow
  FFT::fftsr_destructive_float (block_size, fft_in, fft_out);
}
