// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_EFFECTDECODER_HH
#define SPECTMORPH_EFFECTDECODER_HH

#include "smlivedecoder.hh"
#include "smlivedecodersource.hh"
#include "smmorphoutput.hh"

namespace SpectMorph
{

class EffectDecoder
{
  std::vector<std::unique_ptr<LiveDecoder>> chain_decoders;

  LiveDecoderSource *source;
  float              unison_detune;

public:
  EffectDecoder (LiveDecoderSource *source);
  ~EffectDecoder();

  void set_config (MorphOutput *output);

  void retrigger (int channel, float freq, int midi_velocity, float mix_freq);
  void process (size_t       n_values,
                const float *freq_in,
                const float *freq_mod_in,
                float       *audio_out);
};

}

#endif
