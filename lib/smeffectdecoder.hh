// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_EFFECTDECODER_HH
#define SPECTMORPH_EFFECTDECODER_HH

#include "smlivedecoder.hh"
#include "smlivedecodersource.hh"
#include "smmorphoutput.hh"
#include "smadsrenvelope.hh"
#include "smfilterenvelope.hh"
#include "smladdervcf.hh"
#include "smlinearsmooth.hh"

#include <memory>

namespace SpectMorph
{

class SimpleEnvelope;
class EffectDecoderSource;
class EffectDecoder
{
  MorphOutputModule   *output_module;
  LiveDecoderSource   *original_source;

  bool                 use_skip_source;

  std::unique_ptr<EffectDecoderSource>  skip_source;
  std::unique_ptr<LiveDecoder>          chain_decoder;
  std::unique_ptr<ADSREnvelope>         adsr_envelope;
  std::unique_ptr<SimpleEnvelope>       simple_envelope;

  bool                                  filter_enabled;
  std::function<void()>                 filter_callback;
  FilterEnvelope                        filter_envelope;
  float                                 filter_key_tracking = 0;
  float                                 filter_current_note = 60;
#if 0
  float                                 filter_cutoff;
  float                                 filter_resonance;
#endif
  bool                                  filter_smooth_first;
  LinearSmooth                          filter_cutoff_smooth;
  LinearSmooth                          filter_resonance_smooth;
  float                                 filter_depth_octaves;
  LadderVCFNonLinear                    filter;

public:
  EffectDecoder (MorphOutputModule *output_module, LiveDecoderSource *source);
  ~EffectDecoder();

  void set_config (const MorphOutput::Config *cfg, float mix_freq);

  void retrigger (int channel, float freq, int midi_velocity, float mix_freq);
  void process (size_t       n_values,
                const float *freq_in,
                float       *audio_out);
  void release();
  bool done();

  double time_offset_ms() const;
};

}

#endif
