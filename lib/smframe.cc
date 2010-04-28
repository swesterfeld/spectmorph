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

using Stw::Codec::Frame;

Frame::Frame (size_t frame_size)
  : frame_size (frame_size)
{
  decoded_residue.resize (frame_size);
  decoded_sines.resize (frame_size);
  noise_envelope.resize (256);
}

Frame::Frame (const SpectMorph::AudioBlock& audio_block, size_t frame_size) :
    frame_size (frame_size),
    freqs (audio_block.freqs.begin(), audio_block.freqs.end()),
    phases (audio_block.phases.begin(), audio_block.phases.end()),
    noise_envelope (audio_block.meaning.begin(), audio_block.meaning.end()),
    debug_samples (audio_block.debug_samples)
{
  decoded_residue.resize (frame_size);
  decoded_sines.resize (frame_size);
}
