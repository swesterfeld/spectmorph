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

#include <bse/bsemathsignal.h>

#include <vector>
#include <stdio.h>

using namespace SpectMorph;

using std::vector;

int
main()
{
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

  for (size_t i = 0; i < block_size; i++)
    {
      printf ("%zd %.17g\n", i, samples[i]);
    }
}
