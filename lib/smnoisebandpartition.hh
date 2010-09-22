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


#ifndef SPECTMORPH_NOISE_BAND_PARTITION_HH
#define SPECTMORPH_NOISE_BAND_PARTITION_HH

#include <vector>

namespace SpectMorph
{

class NoiseBandPartition
{
  std::vector<int> band_count;
  std::vector<int> band_from_d;

public:
  NoiseBandPartition (size_t n_bands, size_t n_spectrum_bins, double mix_freq);
  void noise_envelope_to_spectrum (const std::vector<double>& envelope, std::vector<double>& spectrum, double scale);

  size_t n_bands();
  size_t n_spectrum_bins();
};

}

#endif
