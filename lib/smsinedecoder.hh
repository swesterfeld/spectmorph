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


#ifndef __SINEDECODER_HH__
#define __SINEDECODER_HH__

#include "smframe.hh"
#include <vector>

namespace Stw {

namespace Codec {

class SineDecoder
{
public:
  enum Mode {
    MODE_PHASE_SYNC_OVERLAP,
    MODE_TRACKING
  };
private:
  double mix_freq;
  size_t frame_size;
  size_t frame_step;
  std::vector<double> synth_fixed_phase, next_synth_fixed_phase;
  Mode mode;
public:
  SineDecoder (double mix_freq, size_t frame_size, size_t frame_step, Mode mode)
    : mix_freq (mix_freq),
      frame_size (frame_size),
      frame_step (frame_step),
      mode (mode)
  {
  }
  void process (Frame& frame,
                Frame& next_frame,
                const std::vector<double>& window);
};

}

}
#endif
