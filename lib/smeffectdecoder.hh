// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_EFFECTDECODER_HH
#define SPECTMORPH_EFFECTDECODER_HH

#include "smlivedecoder.hh"
#include "smlivedecodersource.hh"
#include "smmorphoutput.hh"
#include "smadsrenvelope.hh"

#include <memory>

namespace SpectMorph
{

class SimpleEnvelope;
class EffectDecoderSource;
class EffectDecoder
{
  LiveDecoderSource   *original_source;

  bool                 use_skip_source;

  std::unique_ptr<EffectDecoderSource>  skip_source;
  std::unique_ptr<LiveDecoder>          chain_decoder;
  std::unique_ptr<ADSREnvelope>         adsr_envelope;
  std::unique_ptr<SimpleEnvelope>       simple_envelope;

public:
  EffectDecoder (LiveDecoderSource *source);
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
