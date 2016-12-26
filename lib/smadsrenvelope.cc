// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smadsrenvelope.hh"
#include "smmath.hh"
#include <algorithm>

using namespace SpectMorph;
using std::max;

int
ADSREnvelope::percent_to_len (float p, float mix_freq) const
{
  const float time_range_ms = 500;
  const float min_time_ms   = 3;

  const float time_ms = max (p / 100 * time_range_ms, min_time_ms);
  const float samples = mix_freq * time_ms / 1000;

  return sm_round_positive (samples);
}

void
ADSREnvelope::set_config (float attack, float decay, float sustain, float release, float mix_freq, bool linear)
{
  attack_delta  = 1.0 / percent_to_len (attack, mix_freq);
  decay_len     = percent_to_len (decay, mix_freq);
  release_len   = percent_to_len (release, mix_freq);

  sustain_level = sustain / 100;

  this->linear = linear;
}

void
ADSREnvelope::retrigger()
{
  level = 0;
  state = State::ATTACK;
  compute_decay_params (decay_len, 1, sustain_level);
}

void
ADSREnvelope::release()
{
  state = State::RELEASE;
  compute_decay_params (release_len, level, 0);
}

void
ADSREnvelope::compute_decay_params (int len, float start_x, float end_x)
{
  params.len = len;

  if (linear)
    {
      // linear
      params.delta = (end_x - start_x) / params.len;
    }
  else
    {
      // exponential

      // compute ADSR envelope until only 1% of original height is left
      const float DELTA = 0.01;

      params.factor = exp (log (DELTA) / params.len);
      params.x = (start_x - end_x) / (1 - DELTA);
      params.offset = start_x - params.x;
    }
}

void
ADSREnvelope::process (size_t n_values, float *values)
{
  for (size_t i = 0; i < n_values; i++)
    {
      if (state == State::ATTACK)
        {
          level += attack_delta;

          if (level >= 1.0)
            {
              state = State::DECAY;
              level = 1.0;
            }
        }
      else if (state == State::DECAY)
        {
          if (linear)
            {
              level += params.delta;
            }
          else
            {
              params.x *= params.factor;
              level = params.offset + params.x;
            }
          if (--params.len <= 0)
            {
              state = State::SUSTAIN;
              level = sustain_level;
            }
        }
      else if (state == State::RELEASE)
        {
          if (linear)
            {
              level += params.delta;
            }
          else
            {
              params.x *= params.factor;
              level = params.offset + params.x;
            }
          if (--params.len <= 0)
            {
              state = State::DONE;
              level = 0;
            }
        }
      values[i] *= level;
    }
}
