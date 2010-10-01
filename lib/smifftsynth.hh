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


#ifndef SPECTMORPH_IFFT_SYNTH_HH
#define SPECTMORPH_IFFT_SYNTH_HH

#include <sys/types.h>
#include <vector>

namespace SpectMorph {

class IFFTSynth
{
  std::vector<float> win_trans;
  int                win_trans_center;

  int                zero_padding;
  size_t             block_size;
  double             mix_freq;

public:
  IFFTSynth (size_t block_size, double mix_freq);

  void render_partial (float *buffer, double freq, double mag, double phase);
  void get_samples (const float *buffer, float *samples, const float *window);
};

}

#endif
