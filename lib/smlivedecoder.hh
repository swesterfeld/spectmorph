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


#ifndef SPECTMORPH_LIVEDECODER_HH
#define SPECTMORPH_LIVEDECODER_HH

#include "smframe.hh"
#include "smwavset.hh"
#include "smsinedecoder.hh"
#include "smnoisedecoder.hh"
#include <vector>

namespace SpectMorph {

class LiveDecoder
{
  WavSet             *smset;
  Audio              *audio;

  SineDecoder        *sine_decoder;
  NoiseDecoder       *noise_decoder;

  size_t              frame_size, frame_step;
  size_t              zero_values_at_start_scaled;
  int                 loop_point;
  float               current_freq;
  float               current_mix_freq;

  size_t              have_samples;
  size_t              pos;
  size_t              env_pos;
  size_t              frame_idx;

  Frame               last_frame;
  std::vector<double> window;
  std::vector<double> samples;

public:
  LiveDecoder (WavSet *smset);

  void retrigger (int channel, float freq, float mix_freq);
  void process (size_t       n_values,
                const float *freq_in,
                const float *freq_mod_in,
                float       *audio_out);

// later:  bool done();
};

}
#endif
