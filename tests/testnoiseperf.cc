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
#include <sys/time.h>

using namespace SpectMorph;
using std::max;
using std::min;
using std::vector;

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

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

  random.set_seed (42);
  for (int i = 0; i < 32; i++)
    audio_block.noise.push_back (random.random_double_range (0.1, 1.0));

  const int RUNS = 20000, REPS = 13;

  vector<float> samples (block_size);
  double min_time[4] = { 1e20, 1e20, 1e20, 1e20 };
  for (int mode = 0; mode < 4; mode++)
    {
      int ifft = (mode == 0) ? 1 : 0;
      int spect = (mode < 3) ? 1 : 0;
      int sse = (mode == 2) ? 0 : 1;
      sm_enable_sse (sse);
      for (int reps = 0; reps < REPS; reps++)
        {
          double start = gettime();
          for (int r = 0; r < RUNS; r++)
            {
              ifft_synth.clear_partials();
              noise_dec.process (audio_block, ifft_synth.fft_buffer(), spect ? NoiseDecoder::FFT_SPECTRUM : NoiseDecoder::DEBUG_NO_OUTPUT);
              if (ifft)
                ifft_synth.get_samples (&samples[0]);
            }
          double end = gettime();
          min_time[mode] = min (min_time[mode], end - start);
        }
    }
  printf ("noise decoder (spectrum gen): %f cycles/sample\n", min_time[3] * 2500.0 * 1000 * 1000 / RUNS / block_size);
  printf ("noise decoder (convolve):     %f cycles/sample\n", (min_time[2] - min_time[3]) * 2500.0 * 1000 * 1000 / RUNS / block_size);
  printf ("noise decoder (convolve/SSE): %f cycles/sample\n", (min_time[1] - min_time[3]) * 2500.0 * 1000 * 1000 / RUNS / block_size);
  printf ("noise decoder (ifft):         %f cycles/sample\n", (min_time[0] - min_time[1]) * 2500.0 * 1000 * 1000 / RUNS / block_size);
}
