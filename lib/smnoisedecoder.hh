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


#ifndef SPECTMORPH_NOISE_DECODER_HH
#define SPECTMORPH_NOISE_DECODER_HH

#include "smframe.hh"
#include "smrandom.hh"
#include "smnoisebandpartition.hh"

namespace SpectMorph
{

/**
 * \brief Decoder for the noise component (stochastic component) of the signal
 */
class NoiseDecoder
{
  double orig_mix_freq;
  double mix_freq;
  Random random_gen;
  NoiseBandPartition *noise_band_partition;

public:
  NoiseDecoder (double orig_mix_freq,
                double mix_freq);
  ~NoiseDecoder();

  void set_seed (int seed);
  void process (const Frame& frame,
                std::vector<float>& decoded_residue);

  size_t preferred_block_size();
};

}

#endif
