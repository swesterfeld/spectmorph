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

#include "smifftsynth.hh"
#include "smmath.hh"

#include <birnet/birnetutils.hh>
#include <bse/bsemathsignal.h>

#include <vector>
#include <stdio.h>
#include <sys/time.h>

using namespace SpectMorph;

using std::vector;
using std::max;
using Birnet::AlignedArray;

double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void
perf_test()
{
  const double mix_freq = 48000;
  const size_t block_size = 1024;

  vector<float> spectrum (block_size);
  vector<float> samples (block_size);
  vector<float> window (block_size);

  IFFTSynth synth (block_size, mix_freq);

  const double freq  = 440;
  const double mag   = 0.1;
  const double phase = 0.7;
  const double clocks_per_sec = 2500.0 * 1000 * 1000;

  size_t RUNS = 1000 * 1000;
  double start, end;

  start = gettime();
  for (int r = 0; r < RUNS; r++)
    synth.render_partial (&spectrum[0], freq, mag, phase);
  end = gettime();

  printf ("render_partial: clocks per sample: %f\n", clocks_per_sec * (end - start) / RUNS / block_size);

  synth.get_samples (&spectrum[0], &samples[0], &window[0]);  // first run may be slower

  RUNS = 100000;
  start = gettime();
  for (int r = 0; r < RUNS; r++)
    synth.get_samples (&spectrum[0], &samples[0], &window[0]);
  end = gettime();

  printf ("get_samples: clocks per sample: %f\n", clocks_per_sec * (end - start) / RUNS / block_size);
}

int
main (int argc, char **argv)
{
  if (argc == 2 && strcmp (argv[1], "perf") == 0)
    {
      perf_test();
      return 0;
    }
  const double mix_freq = 48000;
  const size_t block_size = 1024;

  vector<float> spectrum (block_size);
  vector<float> samples (block_size);
  vector<float> window (block_size);

  for (size_t i = 0; i < window.size(); i++)
    window[i] = bse_window_cos (2.0 * i / block_size - 1.0);

  IFFTSynth synth (block_size, mix_freq);

  const double freq = 4000;
  const double mag = 0.456;
  const double phase = 0.5;

  synth.render_partial (&spectrum[0], freq, mag, phase);
  synth.get_samples (&spectrum[0], &samples[0], &window[0]);

  VectorSinParams vsparams;
  vsparams.mix_freq = mix_freq;
  vsparams.freq = freq;
  vsparams.mag = mag;
  vsparams.phase = phase;
  vsparams.mode = VectorSinParams::REPLACE;

  AlignedArray<float, 16> aligned_decoded_sines (block_size);
  fast_vector_sinf (vsparams, &aligned_decoded_sines[0], &aligned_decoded_sines[block_size]);

  double max_diff = 0;
  for (size_t i = 0; i < block_size; i++)
    {
      max_diff = max (max_diff, double (samples[i]) - aligned_decoded_sines[i] * window[i]);
      //printf ("%zd %.17g %.17g\n", i, samples[i], aligned_decoded_sines[i] * window[i]);
    }
  printf ("# max_diff = %.17g\n", max_diff);

  vsparams.freq = synth.quantized_freq (freq);
  fast_vector_sinf (vsparams, &aligned_decoded_sines[0], &aligned_decoded_sines[block_size]);

  std::fill (spectrum.begin(), spectrum.end(), 0);

  synth.render_partial (&spectrum[0], freq, mag, phase);
  synth.get_samples (&spectrum[0], &samples[0], &window[0]);

  printf ("# qfreq = %.17g\n", vsparams.freq);
  max_diff = 0;
  for (size_t i = 0; i < block_size; i++)
    {
      max_diff = max (max_diff, double (samples[i]) - aligned_decoded_sines[i] * window[i]);
      printf ("%zd %.17g %.17g %.17g\n", i, samples[i], aligned_decoded_sines[i] * window[i], window[i] / window_blackman_harris_92 (2.0 * i / block_size - 1.0));
    }
  printf ("# max_diff = %.17g\n", max_diff);

}
