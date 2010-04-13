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

#ifndef SPECTMORPH_AUDIO_HH
#define SPECTMORPH_AUDIO_HH

#include <vector>

namespace SpectMorph
{

class AudioBlock
{
public:
  std::vector<float> meaning;
  std::vector<float> freqs;
  std::vector<float> mags;
  std::vector<float> phases;
  std::vector<float> original_fft;
  std::vector<float> debug_samples;
};

class Audio
{
public:
  float fundamental_freq;
  float mix_freq;
  float frame_size_ms;
  float frame_step_ms;
  int   zeropad;
  std::vector<AudioBlock> contents;
};

}

#endif
