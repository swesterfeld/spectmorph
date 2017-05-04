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

class EffectDecoderSource;
class EffectDecoder
{
  std::vector<std::unique_ptr<LiveDecoder>> chain_decoders;

  EffectDecoderSource *source;
  float              unison_detune;
  float              unison_gain;

  std::unique_ptr<ADSREnvelope> adsr_envelope;

public:
  EffectDecoder (LiveDecoderSource *source);
  ~EffectDecoder();

  void set_config (MorphOutput *output, float mix_freq);

  void retrigger (int channel, float freq, int midi_velocity, float mix_freq);
  void process (size_t       n_values,
                const float *freq_in,
                const float *freq_mod_in,
                float       *audio_out);
};

}

#endif
