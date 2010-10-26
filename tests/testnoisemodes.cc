/*
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smnoisedecoder.hh"
#include "smifftsynth.hh"
#include "smmain.hh"
#include "smrandom.hh"
#include "smfft.hh"

#include <bse/bsemathsignal.h>

#include <assert.h>

using namespace SpectMorph;
using std::max;
using std::vector;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  double mix_freq = 48000;
  size_t block_size = 1024;

  AudioBlock audio_block;
  NoiseDecoder noise_dec (mix_freq, mix_freq, block_size);
  IFFTSynth ifft_synth (block_size, mix_freq, IFFTSynth::WIN_HANNING);
  Random    random;

  float *samples = FFT::new_array_float (block_size);
  float *win_samples = FFT::new_array_float (block_size);
  float *cos_win_samples = FFT::new_array_float (block_size);
  float *xwin_samples = FFT::new_array_float (block_size);
  float *fft_samples = FFT::new_array_float (block_size);

  random.set_seed (42);
  for (int i = 0; i < 32; i++)
    audio_block.noise.push_back (random.random_double_range (0.1, 1.0));

  // NOISE DEBUG MODE

  noise_dec.set_seed (42);
  noise_dec.process (audio_block, samples, NoiseDecoder::DEBUG_UNWINDOWED);
  for (size_t i = 0; i < block_size / 2; i++)
    {
      win_samples[i] = samples[i + block_size / 2];
      win_samples[i + block_size / 2] = samples[i];
    }
  for (size_t i = 0; i < block_size; i++)
    {
      cos_win_samples[i] = bse_window_cos (2.0 * i / block_size - 1.0) * win_samples[i];
      win_samples[i] *= window_blackman_harris_92 (2.0 * i / block_size - 1.0);
    }
  for (size_t i = 0; i < block_size / 2; i++)
    {
      xwin_samples[i] = win_samples[i + block_size / 2];
      xwin_samples[i + block_size / 2] = win_samples[i];
    }
  float *dbg_spectrum = FFT::new_array_float (block_size);
  FFT::fftar_float (block_size, xwin_samples, dbg_spectrum);

  // NOISE IFFT MODE
  for (int sse = 0; sse < 2; sse++)
    {
      sm_enable_sse (sse);
      ifft_synth.clear_partials();
      noise_dec.set_seed (42);
      noise_dec.process (audio_block, ifft_synth.fft_buffer(), NoiseDecoder::FFT_SPECTRUM);

      vector<float> spectrum (ifft_synth.fft_buffer(), ifft_synth.fft_buffer() + block_size);

      ifft_synth.get_samples (fft_samples);

#if 0
      for (int i = 0; i < block_size; i++)
        printf ("T %d %.17g %.17g\n", i, cos_win_samples[i], fft_samples[i]);

      for (int i = 0; i < block_size; i++)
        printf ("F %d %.17g %.17g\n", i, ifft_synth.fft_buffer()[i], dbg_spectrum[i] / block_size);
#endif
      double t_diff_max = 0;
      for (int i = 0; i < block_size; i++)
        t_diff_max = max (t_diff_max, fabs (cos_win_samples[i] - fft_samples[i]));
      double s_diff_max = 0;
      for (int i = 0; i < block_size; i++)
        s_diff_max = max (s_diff_max, fabs (spectrum[i] - dbg_spectrum[i] / block_size));
      printf ("noise test: sse=%d t_diff_max=%.17g\n", sse, t_diff_max);
      printf ("noise test: sse=%d s_diff_max=%.17g\n", sse, s_diff_max);
      assert (t_diff_max < 3e-5);
      assert (s_diff_max < 1e-7);
    }
}
