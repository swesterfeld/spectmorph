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

  const int RUNS = 100000;

  vector<float> samples (block_size);
  for (int mode = 0; mode < 3; mode++)
    {
      int ifft = (mode == 0) ? 1 : 0;
      int spect = (mode < 2) ? 1 : 0;
      double start = gettime();
      for (int r = 0; r < RUNS; r++)
      {
        ifft_synth.clear_partials();
        noise_dec.process (audio_block, ifft_synth.fft_buffer(), spect ? NoiseDecoder::FFT_SPECTRUM : NoiseDecoder::DEBUG_NO_OUTPUT);
        if (ifft)
          ifft_synth.get_samples (&samples[0]);
      }
      double end = gettime();
      printf ("noise decoder (IFFT=%d; SPECT=%d): %f cycles/sample\n", ifft, spect, (end - start) * 2500.0 * 1000 * 1000 / RUNS / block_size);
    }
}
