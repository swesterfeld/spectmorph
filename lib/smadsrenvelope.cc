// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smadsrenvelope.hh"
#include "smmath.hh"
#include <bse/bsemathsignal.hh>
#include <algorithm>

using namespace SpectMorph;
using std::max;

static float
exp_percent (float p, float min_out, float max_out, float slope)
{
  /* exponential curve from 0 to 1 with configurable slope */
  const double x = (pow (2, (p / 100.) * slope) - 1) / (pow (2, slope) - 1);

  /* rescale to interval [min_out, max_out] */
  return x * (max_out - min_out) + min_out;
}

void
ADSREnvelope::set_config (float attack, float decay, float sustain, float release, float mix_freq, bool linear)
{
  const float samples_per_ms = mix_freq / 1000;

  attack_delta  = 1.0 / (exp_percent (attack, 2, 5000, 5) * samples_per_ms);

  // FIXME: adapt max length
  decay_len     = sm_round_positive (exp_percent (decay, 2, 1000, 5) * samples_per_ms);
  release_len   = sm_round_positive (exp_percent (release, 2, 1000, 5) * samples_per_ms);

  sustain_level = exp_percent (sustain, 0, 1, /* slope */ 5);
  // printf ("%.3f dB\n", bse_db_from_factor (sustain_level, -96));

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
  if (linear)
    {
      // linear
      params.len   = len;
      params.delta = (end_x - start_x) / params.len;

      params.factor = 1; // not used
    }
  else
    {
      // exponential

      // reach zero approximately if 1% / -40dB of original height is left
      const float RATIO = 0.01;

      /* compute iterative exponential decay parameters from inputs:
       *
       *   - len:           half life time
       *   - RATIO:         target ratio (when should we reach zero)
       *   - start_x/end_x: level at start/end of the decay slope
       *
       * iterative computation of next value (should be done params.len times):
       *
       *    value = value * params.factor + params.delta
       */
      const double f = -log ((RATIO + 1)/(RATIO + 0.5))/len;

      params.len    = -log ((RATIO + 1) / RATIO)/f;
      params.factor = exp (f);
      params.delta  = (end_x - RATIO * (start_x - end_x)) * (1 - params.factor);
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
              level = level * params.factor + params.delta;
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
              level = level * params.factor + params.delta;
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

void
ADSREnvelope::test_decay (int len, float start_x, float end_x)
{
  linear = false;

  compute_decay_params (len, start_x, end_x);

  level = start_x;
  for (int i = 0; i < params.len + len * 5; i++)
    {
      level = level * params.factor + params.delta;
      printf ("%d %f %f\n", i, level, i < params.len ? start_x : end_x);
    }
}
