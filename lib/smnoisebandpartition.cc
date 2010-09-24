/*
 * Copyright (C) 2009-2010 Stefan Westerfeld
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

#include <math.h>
#include <assert.h>
#include <stdio.h>

#include "smnoisebandpartition.hh"

using namespace SpectMorph;
using std::vector;

static double
mel_to_hz (double mel)
{
  return 700 * (exp (mel / 1127.0) - 1);
}

NoiseBandPartition::NoiseBandPartition (size_t n_bands, size_t n_spectrum_bins, double mix_freq) :
  band_count (n_bands),
  band_from_d (n_spectrum_bins)
{
  int d = 0;
  /* assign each d to a band */
  std::fill (band_from_d.begin(), band_from_d.end(), -1);
  for (size_t band = 0; band < n_bands; band++)
    {
      double mel_low = 30 + 4000.0 / n_bands * band;
      double mel_high = 30 + 4000.0 / n_bands * (band + 1);
      double hz_low = mel_to_hz (mel_low);
      double hz_high = mel_to_hz (mel_high);

      /* skip frequencies which are too low to be in lowest band */
      double f_hz = mix_freq / 2.0 * d / n_spectrum_bins;
      if (band == 0)
        {
          while (f_hz < hz_low)
            {
              d += 2;
              f_hz = mix_freq / 2.0 * d / n_spectrum_bins;
            }
        }
      while (f_hz < hz_high && d < n_spectrum_bins)
        {
          if (d < band_from_d.size())
            {
              band_from_d[d] = band;
              band_from_d[d + 1] = band;
            }
          d += 2;
          f_hz = mix_freq / 2.0 * d / n_spectrum_bins;
        }
    }
  /* count bins per band */
  for (int d = 0; d < n_spectrum_bins; d += 2)
    {
      int b = band_from_d[d];
      if (b != -1)
        {
          assert (b >= 0 && b < n_bands);
          band_count[b]++;
        }
    }
}

size_t
NoiseBandPartition::n_bands()
{
  return band_count.size();
}

size_t
NoiseBandPartition::n_spectrum_bins()
{
  return band_from_d.size();
}

void
NoiseBandPartition::noise_envelope_to_spectrum (const vector<double>& envelope, vector<double>& spectrum, double scale)
{
  assert (spectrum.size() == n_spectrum_bins());
  assert (envelope.size() == n_bands());

  double band_value[1 + n_bands()];

  /* precompute spectrum values for all bands and -1 (which means no band has been assigned) */
  band_value[0] = 0;
  for (size_t b = 0; b < n_bands(); b++)
    band_value[1 + b] = sqrt (envelope[b] / band_count[b]) * scale;

  for (size_t d = 0; d < spectrum.size(); d += 2)
    {
      int b = band_from_d[d] + 1;

      spectrum[d] = band_value[b];
      spectrum[d+1] = 0;
    }
}
