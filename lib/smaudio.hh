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

#include <bse/bsecxxplugin.hh>
#include <vector>

#include "smgenericin.hh"

namespace SpectMorph
{

/**
 * \brief Block of audio data, encoded in SpectMorph parametric format
 *
 * This represents a single analysis frame, usually containing sine waves in freqs
 * and phases, and a noise envelope for everything that remained after subtracting
 * the sine waves. The parameters original_fft and debug_samples are optional and
 * are used for debugging only.
 */
class AudioBlock
{
public:
  std::vector<float> noise;          //!< noise envelope, representing the original signal minus sine components
  std::vector<float> freqs;          //!< frequencies of the sine components of this frame
  std::vector<float> phases;         //!< phases and magnitude of the sine components, represented as sin and cos magnitude
  std::vector<float> original_fft;   //!< original zeropadded FFT data - for debugging only
  std::vector<float> debug_samples;  //!< original audio samples for this frame - for debugging only
};

enum AudioLoadOptions
{
  AUDIO_LOAD_DEBUG,
  AUDIO_SKIP_DEBUG
};

/**
 * \brief Audio sample containing many blocks
 *
 * This class contains the information the SpectMorph::Encoder creates for a wav file. The
 * time dependant parameters are stored in contents, as a vector of audio frames; the
 * parameters that are the same for all frames are stored in this class.
 */
class Audio
{
public:
  Audio();

  float fundamental_freq;            //!< fundamental frequency (note which was encoded), or 0 if not available
  float mix_freq;                    //!< mix freq (sampling rate) of the original audio data
  float frame_size_ms;               //!< length of each audio frame in milliseconds
  float frame_step_ms;               //!< stepping of the audio frames in milliseconds
  float attack_start_ms;             //!< start of attack in milliseconds
  float attack_end_ms;               //!< end of attack in milliseconds
  int   zeropad;                     //!< FFT zeropadding used during analysis
  int   loop_point;                  //!< loop point to be used during sustain phase of playback
  int   zero_values_at_start;        //!< number of zero values added by encoder (strip during decoding)
  std::vector<AudioBlock> contents;  //!< the actual frame data

  BseErrorType load (const std::string& filename, AudioLoadOptions load_options = AUDIO_LOAD_DEBUG);
  BseErrorType load (SpectMorph::GenericIn *file, AudioLoadOptions load_options = AUDIO_LOAD_DEBUG);
  BseErrorType save (const std::string& filename);
};

}

#endif
