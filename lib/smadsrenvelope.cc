// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smadsrenvelope.hh"
#include "smmath.hh"
#include <assert.h>
#include <algorithm>

using namespace SpectMorph;
using std::max;
using std::min;

static float
exp_percent (float p, float min_out, float max_out, float slope)
{
  /* exponential curve from 0 to 1 with configurable slope */
  const double x = (pow (2, (p / 100.) * slope) - 1) / (pow (2, slope) - 1);

  /* rescale to interval [min_out, max_out] */
  return x * (max_out - min_out) + min_out;
}

static float
xparam_percent (float p, float min_out, float max_out, float slope)
{
  /* rescale xparam function to interval [min_out, max_out] */
  return sm_xparam (p / 100.0, slope) * (max_out - min_out) + min_out;
}

void
ADSREnvelope::set_config (float attack, float decay, float sustain, float release, float mix_freq)
{
  const float samples_per_ms = mix_freq / 1000;

  attack_len    = sm_round_positive (xparam_percent (attack, 2, 5000, 3) * samples_per_ms);
  decay_len     = sm_round_positive (exp_percent (decay, 2, 1000, 5) * samples_per_ms);
  release_len   = sm_round_positive (exp_percent (release, 2, 200, 3) * samples_per_ms);

  sustain_level = exp_percent (sustain, 0, 1, /* slope */ 5);
  if (0)
    {
      printf ("%.2f ms -  %.2f ms  -  %.3f dB  -  %.2f ms\n",
          attack_len / samples_per_ms,
          decay_len / samples_per_ms,
          db_from_factor (sustain_level, -96),
          release_len / samples_per_ms );
    }
}

void
ADSREnvelope::retrigger()
{
  level = 0;
  state = State::ATTACK;

  compute_slope_params (attack_len, 0, 1, State::ATTACK);
}

void
ADSREnvelope::release()
{
  state = State::RELEASE;

  compute_slope_params (release_len, level, 0, State::RELEASE);
}

bool
ADSREnvelope::done() const
{
  return (state == State::DONE);
}

void
ADSREnvelope::compute_slope_params (int len, float start_x, float end_x, State param_state)
{
  params.end    = end_x;

  if (param_state == State::ATTACK)
    {
      // linear
      params.len    = len;
      params.delta  = (end_x - start_x) / params.len;
      params.linear = true;

      params.factor = 1; // not used
    }
  else
    {
      assert (param_state == State::DECAY || param_state == State::RELEASE);

      // exponential

      /* true exponential decay doesn't ever reach zero; therefore we need to
       * fade out early
       *
       * during RELEASE, it is very important that this process terminates
       * quickly, because after the note has ended, we don't want to waste
       * cpu cycles for barely audible tail; so we choose a very crude
       * approximation in this case (and a good approximation for DECAY)
       */
      double RATIO;
      if (param_state == State::DECAY)
        RATIO = 0.0001; // -80dB  or   0.01% of the original height
      else
        RATIO = 0.1;    // -20dB  or  10.00% of the original height

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
      params.linear = false;
    }
}

size_t
ADSREnvelope::process_params (size_t n_values, float *values)
{
  n_values = min<int> (n_values, params.len);

  if (params.linear)
    {
      for (size_t i = 0; i < n_values; i++)
        {
          // params.factor == 1 -> avoid multiplication for linear case
          level     += params.delta;
          values[i] *= level;
        }
    }
  else
    {
      for (size_t i = 0; i < n_values; i++)
        {
          level = level * params.factor + params.delta;
          values[i] *= level;
        }
    }
  params.len -= n_values;

  if (!params.len)
    level = params.end;

  return n_values;
}

void
ADSREnvelope::process (size_t n_values, float *values)
{
  size_t i = 0;
  while (i < n_values)
    {
      if (state == State::ATTACK)
        {
          i += process_params (n_values - i, values + i);

          if (!params.len)
            {
              compute_slope_params (decay_len, 1, sustain_level, State::DECAY);

              state = State::DECAY;
            }
        }
      else if (state == State::DECAY)
        {
          i += process_params (n_values - i, values + i);

          if (!params.len)
            state = State::SUSTAIN;
        }
      else if (state == State::RELEASE)
        {
          i += process_params (n_values - i, values + i);

          if (!params.len)
            state = State::DONE;
        }
      else
        {
          // constant value (sustain or done)
          while (i < n_values)
            values[i++] *= level;
        }
    }
}

void
ADSREnvelope::test_decay (int len, float start_x, float end_x)
{
  compute_slope_params (len, start_x, end_x, State::DECAY);

  level = start_x;
  for (int i = 0; i < params.len + len * 5; i++)
    {
      level = level * params.factor + params.delta;
      printf ("%d %f %f\n", i, level, i < params.len ? start_x : end_x);
    }
}
