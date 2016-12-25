// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_ADSR_ENVELOPE_HH
#define SPECTMORPH_ADSR_ENVELOPE_HH

#include <stddef.h>

namespace SpectMorph {

class ADSREnvelope
{
  enum class State { ATTACK, DECAY, SUSTAIN };

  State state;
  float level;
  float attack_delta;
  float decay_delta;
  float sustain_level;

  float percent2delta (float p, float mix_freq) const;
public:
  void set_config (float attack, float decay, float sustain, float release, float mix_freq);
  void retrigger();
  void process (size_t n_values, float *values);
};

}

#endif
