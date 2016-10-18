// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_EFFECTDECODER_HH
#define SPECTMORPH_EFFECTDECODER_HH

#include "smlivedecoder.hh"
#include "smlivedecodersource.hh"

namespace SpectMorph
{

class EffectDecoder
{
  LiveDecoder *chain;
  LiveDecoder *chain2;
  LiveDecoder *chain3;

  bool chorus_active;

public:
  EffectDecoder (LiveDecoderSource *source);
  ~EffectDecoder();

  void enable_noise (bool en);
  void enable_sines (bool es);
  void enable_chorus (bool ec);

  void retrigger (int channel, float freq, int midi_velocity, float mix_freq);
  void process (size_t       n_values,
                const float *freq_in,
                const float *freq_mod_in,
                float       *audio_out);
};

}

#endif
