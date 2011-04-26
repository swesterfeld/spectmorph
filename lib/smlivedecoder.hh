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


#ifndef SPECTMORPH_LIVEDECODER_HH
#define SPECTMORPH_LIVEDECODER_HH

#include "smwavset.hh"
#include "smsinedecoder.hh"
#include "smnoisedecoder.hh"
#include "smlivedecodersource.hh"
#include "smpolyphaseinter.hh"
#include <birnet/birnet.hh>
#include <vector>

namespace SpectMorph {

class LiveDecoder
{
  struct PartialState
  {
    float freq;
    float phase;
  };
  std::vector<PartialState> pstate[2], *last_pstate;

  WavSet             *smset;
  Audio              *audio;

  IFFTSynth          *ifft_synth;
  NoiseDecoder       *noise_decoder;
  LiveDecoderSource  *source;
  PolyPhaseInter     *pp_inter;

  bool                sines_enabled;
  bool                noise_enabled;
  bool                debug_fft_perf_enabled;
  bool                original_samples_enabled;

  size_t              frame_size, frame_step;
  size_t              zero_values_at_start_scaled;
  size_t              loop_start_scaled;
  size_t              loop_end_scaled;
  int                 loop_point;
  float               current_freq;
  float               current_mix_freq;
  float               latency_ms;

  size_t              have_samples;
  size_t              block_size;
  size_t              pos;
  size_t              env_pos;
  size_t              frame_idx;
  double              original_sample_pos;

  size_t              latency_zero_samples;
  int                 noise_seed;

  Birnet::AlignedArray<float,16> *sse_samples;
public:
  LiveDecoder (WavSet *smset);
  LiveDecoder (LiveDecoderSource *source);
  ~LiveDecoder();

  void enable_noise (bool ne);
  void enable_sines (bool se);
  void enable_debug_fft_perf (bool dfp);
  void enable_original_samples (bool eos);
  void set_noise_seed (int seed);
  void set_latency_ms (float latency_ms);

  void precompute_tables (float mix_freq);
  void retrigger (int channel, float freq, int midi_velocity, float mix_freq);
  void process (size_t       n_values,
                const float *freq_in,
                const float *freq_mod_in,
                float       *audio_out);

  static size_t compute_loop_frame_index (size_t index, Audio *audio);
// later:  bool done();
};

}
#endif
