// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphlfo.hh"

#include <math.h>
#include <glib.h>

using namespace SpectMorph;

// copypaste this from smmorphlfomodule.hh
struct LFOState
{
  double phase              = 0;
  double last_random_value  = 0;
  double random_value       = 0;
  double value              = 0;
};

// copypaste this from smmorphlfomodule.hh
float start_phase = 0;
float frequency = 1;
float depth = 1;
float center = 0;
MorphLFO::WaveType wave_type = MorphLFO::WAVE_SINE;

// copypaste this from smmorphlfomodule.cc
static double
normalize_phase (double phase)
{
  return fmod (phase + 1, 1);
}

// copypaste this from smmorphlfomodule.cc
void
update_lfo_value (LFOState& state, double time_ms)
{
  state.phase += time_ms / 1000 * frequency;
  if (state.phase > 1)
    {
      state.last_random_value = state.random_value;
      state.random_value = g_random_double_range (-1, 1);
    }
  state.phase = normalize_phase (state.phase);

  if (wave_type == MorphLFO::WAVE_SINE)
    {
      state.value = sin (state.phase * M_PI * 2);
    }
  else if (wave_type == MorphLFO::WAVE_TRIANGLE)
    {
      if (state.phase < 0.25)
        {
          state.value = 4 * state.phase;
        }
      else if (state.phase < 0.75)
        {
          state.value = (state.phase - 0.5) * -4;
        }
      else
        {
          state.value = 4 * (state.phase - 1);
        }
    }
  else if (wave_type == MorphLFO::WAVE_SAW_UP)
    {
      state.value = -1 + 2 * state.phase;
    }
  else if (wave_type == MorphLFO::WAVE_SAW_DOWN)
    {
      state.value = 1 - 2 * state.phase;
    }
  else if (wave_type == MorphLFO::WAVE_SQUARE)
    {
      if (state.phase < 0.5)
        state.value = -1;
      else
        state.value = 1;
    }
  else if (wave_type == MorphLFO::WAVE_RANDOM_SH)
    {
      state.value = state.random_value;
    }
  else if (wave_type == MorphLFO::WAVE_RANDOM_LINEAR)
    {
      state.value = state.last_random_value * (1 - state.phase) + state.random_value * state.phase;
    }
  else
    {
      g_assert_not_reached();
    }

  state.value = state.value * depth + center;
  state.value = CLAMP (state.value, -1.0, 1.0);
}

// copypaste this from smmorphlfomodule.cc
void
restart_lfo (LFOState& state)
{
  state.phase = normalize_phase (start_phase / 360);
  state.last_random_value = g_random_double_range (-1, 1);
  state.random_value = g_random_double_range (-1, 1);
  /* compute initial value */
  update_lfo_value (state, 0);
}

int
main()
{
  LFOState lfo_state;
  restart_lfo (lfo_state);
  for (int t = 0; t < 20000; t++)
    {
      printf ("%d %f\n", t, lfo_state.value);
      update_lfo_value (lfo_state, 1);
    }
}
