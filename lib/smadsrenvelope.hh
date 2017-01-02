// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_ADSR_ENVELOPE_HH
#define SPECTMORPH_ADSR_ENVELOPE_HH

#include <stddef.h>

namespace SpectMorph {

class ADSREnvelope
{
  enum class State { ATTACK, DECAY, SUSTAIN, RELEASE, DONE };

  State state;
  float level;
  float attack_delta;
  float decay_len;
  float release_len;
  float sustain_level;
  bool linear;

  struct DecayParams {
    int len;

    float factor;     // exponential slope only
    float delta;      // exponential slope & linear slope
  } params;

  void compute_decay_params (int len, float start_x, float end_x);
public:
  void set_config (float attack, float decay, float sustain, float release, float mix_freq, bool linear);
  void retrigger();
  void release();
  void process (size_t n_values, float *values);

  // test only
  void test_decay (int len, float start_x, float end_x);
};

}

#endif
