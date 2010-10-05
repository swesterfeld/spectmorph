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

class IFFTSynthTable;

class IFFTSynth
{
  IFFTSynthTable    *table;

  int                zero_padding;
  size_t             block_size;
  double             mix_freq;
  double             freq256_factor;

  float             *fft_in;
  float             *fft_out;
  float             *win_scale;

public:
  enum WindowType { WIN_BLACKMAN_HARRIS_92, WIN_HANNING };

  IFFTSynth (size_t block_size, double mix_freq, WindowType win_type);
  ~IFFTSynth();

  void clear_partials();
  void render_partial (double freq, double mag, double phase);
  void get_samples (float *samples);

  double quantized_freq (double freq);
};

}

#endif
