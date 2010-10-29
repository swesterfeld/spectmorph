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


#ifndef SPECTMORPH_LIVEDECODER_SOURCE_HH
#define SPECTMORPH_LIVEDECODER_SOURCE_HH

#include "smaudio.hh"

namespace SpectMorph {

class LiveDecoderSource
{
public:
  virtual void retrigger (int channel, float freq, int midi_velocity, float mix_freq) = 0;
  virtual Audio *audio() = 0;
  virtual AudioBlock *audio_block (size_t index) = 0;
  virtual ~LiveDecoderSource();
};

}
#endif
