// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smadsrenvelope.hh"
#include <algorithm>

using namespace SpectMorph;
using std::max;

float
ADSREnvelope::percent2delta (float p, float mix_freq) const
{
  const float time_range_ms = 500;
  const float min_time_ms   = 3;

  const float time_ms = max (p / 100 * time_range_ms, min_time_ms);
  const float samples = mix_freq * time_ms / 1000;

  return 1 / samples;
}

void
ADSREnvelope::set_config (float attack, float decay, float sustain, float release, float mix_freq)
{
  attack_delta = percent2delta (attack, mix_freq);
  decay_delta  = percent2delta (decay, mix_freq);

  sustain_level = sustain / 100;
}

void
ADSREnvelope::retrigger()
{
  level = 0;
  state = State::ATTACK;
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
          level -= decay_delta;

          if (level <= sustain_level)
            {
              state = State::SUSTAIN;
              level = sustain_level;
            }
        }
      values[i] *= level;
    }
}


