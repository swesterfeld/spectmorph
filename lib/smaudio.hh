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

/**
 * Block of audio data, encoded in SpectMorph parametric format
 */
class AudioBlock
{
public:
  std::vector<float> noise;          //!< noise envelope, representing the original signal minus sine components
  std::vector<float> freqs;          //!< frequencies of the sine components of this frame
  std::vector<float> phases;         //!< phases of the sine components, represented as sine and cosine magnitude
  std::vector<float> original_fft;   //!< original zeropadded FFT data - for debugging only
  std::vector<float> debug_samples;  //!< original audio samples for this frame - for debugging only
};

/**
 * Audio sample containing many blocks
 */
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
