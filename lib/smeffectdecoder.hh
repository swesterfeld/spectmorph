// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_EFFECTDECODER_HH
#define SPECTMORPH_EFFECTDECODER_HH

#include "smlivedecoder.hh"
#include "smlivedecodersource.hh"
#include "smmorphoutput.hh"
#include "smadsrenvelope.hh"
#include "smlivedecoderfilter.hh"

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

  bool                                  filter_enabled = false;
  LiveDecoderFilter                     live_decoder_filter;

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
