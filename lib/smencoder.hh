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

#ifndef SPECTMORPH_ENCODER_HH
#define SPECTMORPH_ENCODER_HH

#include <sys/types.h>
#include <vector>
#include <string>
#include <bse/gsldatautils.h>

#include "smaudio.hh"

namespace SpectMorph
{

/**
 * \brief Encoder parameters
 *
 * This struct contains parameters that configure the encoder algorithm.
 */
struct EncoderParams
{
  /** sample rate of the original audio file */
  float   mix_freq;         

  /** step size for analysis frames in milliseconds */
  float   frame_step_ms;

  /** size of one analysis frame in milliseconds */
  float   frame_size_ms;

  /** lower bound for the amount of zero padding used during analysis */
  int     zeropad;

  /** size of the frame step in samples */
  size_t  frame_step;

  /** size of the analysis frame in samples */
  size_t  frame_size;

  /** analysis block size in samples, must be the smallest power of N greater than frame_size */
  size_t  block_size;

  /** user defined fundamental freq */
  double  fundamental_freq;
};

/// @cond
struct Tracksel {
  size_t   frame;
  size_t   d;         /* FFT position */
  double   freq;
  double   mag;
  double   mag2;      /* magnitude in dB */
  double   smag;      /* sine amplitude */
  double   cmag;      /* cosine amplitude */
  bool     is_harmonic;
  Tracksel *prev, *next;
};
/// @endcond

/**
 * \brief Encoder producing SpectMorph parametric data from sample data
 *
 * The encoder needs to perform a number of analysis steps to get from the input
 * signal to a parametric representation (which is built in audio_blocks). At the
 * moment, this process needs to be controlled by the caller, but a simpler
 * interface should be added.
 */
class Encoder
{
  EncoderParams enc_params;

  bool check_harmonic (double freq, double& new_freq, double mix_freq);

  std::vector< std::vector<Tracksel> > frame_tracksels; //!< Analog to Canny Algorithms edgels - only used internally

  struct Attack
  {
    double attack_start_ms;
    double attack_end_ms;
  };
  double attack_error (const std::vector< std::vector<double> >& unscaled_signal, const std::vector<float>& window, const Attack& attack, std::vector<double>& out_scale);

public:
  std::vector<AudioBlock>              audio_blocks;    //!< current state, and end result of the encoding algorithm
  Attack                               optimal_attack;
  size_t                               zero_values_at_start;
  size_t                               sample_count;

  Encoder (const EncoderParams& enc_params);

  // single encoder steps:
  void compute_stft (GslDataHandle *dhandle, int channel, const std::vector<float>& window);
  void search_local_maxima();
  void link_partials();
  void validate_partials();
  void optimize_partials (const std::vector<float>& window, int optimization_level);
  void spectral_subtract (const std::vector<float>& window);
  void approx_noise();
  void compute_attack_params (const std::vector<float>& window);

  // all-in-one encoding function:
  void encode (GslDataHandle *dhandle, int channel, const std::vector<float>& window, int optimization_level, bool attack);

  void save (const std::string& filename, double fundamental_freq);
};

}

#endif
