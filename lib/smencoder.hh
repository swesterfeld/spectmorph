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

struct EncoderParams
{
  float   mix_freq;         /* mix_freq of the original audio file */
  float   frame_step_ms;    /* step size for analysis frames */
  float   frame_size_ms;    /* size of one analysis frame */
  int     zeropad;          /* lower bound for zero padding during analysis */
  size_t  frame_step;       /* frame step */
  size_t  frame_size;       /* frame size */
  size_t  block_size;       /* analysis block size */
  double  fundamental_freq; /* user defined fundamental freq */
};

struct Tracksel {
  size_t   frame;
  size_t   d;         /* FFT position */
  double   freq;
  double   mag;
  double   mag2;      /* magnitude in dB */
  double   phasea;    /* sine amplitude */
  double   phaseb;    /* cosine amplitude */
  bool     is_harmonic;
  Tracksel *prev, *next;
};

class Encoder
{
  EncoderParams enc_params;

  bool check_harmonic (double freq, double& new_freq, double mix_freq);

public:
  Encoder (const EncoderParams& enc_params)
  {
    this->enc_params = enc_params;
  }
  std::vector<AudioBlock> audio_blocks;

  void compute_stft (GslDataHandle *dhandle, const std::vector<float>& window);
  void search_local_maxima (std::vector< std::vector<Tracksel> >& frame_tracksels);
  void link_partials (std::vector< std::vector<Tracksel> >& frame_tracksels);
  void validate_partials (std::vector< std::vector<Tracksel> >& frame_tracksels);
  void optimize_partials (const std::vector<float>& window, bool optimize);
  void spectral_subtract (const std::vector<float>& window);
  void approx_noise();
  void save (const std::string& filename, double fundamental_freq);
};

}

#endif
