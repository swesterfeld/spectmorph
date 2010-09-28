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

#include "smframe.hh"

using SpectMorph::Frame;

/**
 * \brief Constructor for an empty frame
 *
 * This constructor sets up all data members of frame for an empty frame; it is intended
 * to be used when constructing new data from existing data.
 */
Frame::Frame()
{
  noise_envelope.resize (32);
}

/**
 * \brief Constructor for a frame from an AudioBlock
 *
 * This constructor copies all parameters from the AudioBlock. The frame_size is used to
 * setup the decoder buffers correctly.
 */
Frame::Frame (const SpectMorph::AudioBlock& audio_block) :
    freqs (audio_block.freqs.begin(), audio_block.freqs.end()),
    phases (audio_block.phases.begin(), audio_block.phases.end()),
    noise_envelope (audio_block.noise.begin(), audio_block.noise.end()),
    debug_samples (audio_block.debug_samples)
{
}

/**
 * computes the magnitude of a partial (complex abs)
 * \param partial the partial (numbered in the same way as the freq index)
 */
double
Frame::magnitude (size_t partial) const
{
  double a = phases[partial * 2];
  double b = phases[partial * 2 + 1];

  return sqrt (a * a + b * b);
}
