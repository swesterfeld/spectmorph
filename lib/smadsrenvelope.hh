// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_ADSR_ENVELOPE_HH
#define SPECTMORPH_ADSR_ENVELOPE_HH

#include <stddef.h>

namespace SpectMorph {

class ADSREnvelope
{
  enum class State { ATTACK, DECAY, SUSTAIN, RELEASE, DONE };

  State state = State::DONE;
  double level;
  int attack_len;
  int decay_len;
  int release_len;
  float sustain_level;

  struct SlopeParams {
    int len;

    double factor;     // exponential slope only
    double delta;      // exponential slope & linear slope
    double end;

    bool linear;
  } params;

  size_t process_params (size_t len, float *values);
  void compute_slope_params (int len, float start_x, float end_x, State param_state);
public:
  void set_config (float attack, float decay, float sustain, float release, float mix_freq);
  void retrigger();
  void release();
  bool done() const;
  void process (size_t n_values, float *values);

  // test only
  void test_decay (int len, float start_x, float end_x);
};

}

#endif
