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


#ifndef SPECTMORPH_FRAME_HH
#define SPECTMORPH_FRAME_HH

#include "smaudio.hh"

namespace SpectMorph {

/**
 * \brief This class represents a single frame to be decoded.
 */
class Frame
{
  size_t frame_size;
public:
  Frame (size_t frame_size);
  Frame (const SpectMorph::AudioBlock& audio_block, size_t frame_size);

  std::vector<double> freqs;                 //!< frequencies of the sine waves in this frame
  std::vector<double> phases;                //!< magnitude and phase of the sine waves, as sin and cos magnitude
  std::vector<double> noise_envelope;        //!< noise envelope representing residue spectrum without sines
  std::vector<float>  debug_samples;         //!< original sample values - debugging only

  double magnitude (size_t partial) const;
};

}

#endif
