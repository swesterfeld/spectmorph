// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LIVEDECODER_HH
#define SPECTMORPH_LIVEDECODER_HH

#include "smwavset.hh"
#include "smsinedecoder.hh"
#include "smnoisedecoder.hh"
#include "smlivedecodersource.hh"
#include "smpolyphaseinter.hh"
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
  bool                loop_enabled;

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
  double              original_samples_norm_factor;

  size_t              latency_zero_samples;
  int                 noise_seed;

  Rapicorn::AlignedArray<float,16> *sse_samples;

  Audio::LoopType     get_loop_type();

public:
  LiveDecoder (WavSet *smset);
  LiveDecoder (LiveDecoderSource *source);
  ~LiveDecoder();

  void enable_noise (bool ne);
  void enable_sines (bool se);
  void enable_debug_fft_perf (bool dfp);
  void enable_original_samples (bool eos);
  void enable_loop (bool eloop);
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
