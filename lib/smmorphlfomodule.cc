// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphlfomodule.hh"
#include "smmorphlfo.hh"
#include "smmorphplan.hh"
#include "smwavsetrepo.hh"
#include "smleakdebugger.hh"
#include "smmorphplanvoice.hh"
#include "smmorphplansynth.hh"
#include "smmath.hh"

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::MorphLFOModule");

MorphLFOModule::MorphLFOModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  leak_debugger.add (this);

  shared_state = NULL;
}

MorphLFOModule::~MorphLFOModule()
{
  leak_debugger.del (this);
}

static double
normalize_phase (double phase)
{
  return fmod (phase + 1, 1);
}

MorphModuleSharedState *
MorphLFOModule::create_shared_state()
{
  return new SharedState();
}

void
MorphLFOModule::set_shared_state (MorphModuleSharedState *new_shared_state)
{
  shared_state = dynamic_cast<SharedState *> (new_shared_state);
  assert (shared_state);
}

void
MorphLFOModule::set_config (const MorphOperatorConfig *op_cfg)
{
  cfg = dynamic_cast<const MorphLFO::Config *> (op_cfg);

  if (!shared_state->initialized)
    {
      restart_lfo (shared_state->global_lfo_state, /* start from zero time */ TimeInfo());
      shared_state->initialized = true;
    }
}

float
MorphLFOModule::value()
{
  TimeInfo time = time_info();

  if (cfg->sync_voices)
    {
      auto lfo_state = shared_state->global_lfo_state;
      update_lfo_value (lfo_state, time);
      set_notify_value (lfo_state.value);

      return lfo_state.value;
    }
  else
    {
      update_lfo_value (local_lfo_state, time);
      set_notify_value (local_lfo_state.value);

      return local_lfo_state.value;
    }
}

void
MorphLFOModule::reset_value (const TimeInfo& time_info)
{
  restart_lfo (local_lfo_state, time_info);
}

void
MorphLFOModule::restart_lfo (LFOState& state, const TimeInfo& time_info)
{
  state = LFOState(); /* reset to defaults */
  state.last_random_value = random_gen()->random_double_range (-1, 1);
  state.random_value = random_gen()->random_double_range (-1, 1);
  /* compute initial value */
  TimeInfo zero_time;
  update_lfo_value (state, zero_time);
  state.last_time_ms = time_info.time_ms;
  state.last_ppq_pos = time_info.ppq_pos;
}

void
MorphLFOModule::update_lfo_value (LFOState& state, const TimeInfo& time_info)
{
  if (!cfg->beat_sync)
    {
      if (time_info.time_ms > state.last_time_ms)
        state.raw_phase += (time_info.time_ms - state.last_time_ms) / 1000 * cfg->frequency;
      state.last_time_ms = time_info.time_ms;
    }
  else
    {
      if (time_info.ppq_pos > state.last_ppq_pos)
        {
          /* If sync_voices is disabled, each note should have its own phase.
           *
           * To compute this, we want to know how long the note has been playing.
           * ppq_count tries to do this even in presence of backward jumps as
           * they are caused by loops. There is a small error here, during
           * jumps, but the result should be acceptable.
           */
          state.ppq_count += time_info.ppq_pos - state.last_ppq_pos;
        }
      state.last_ppq_pos = time_info.ppq_pos;

      double factor = pow (2, (MorphLFO::NOTE_1_4 - cfg->note)); // <- tempo is relative to quarter notes
      switch (cfg->note_mode)
      {
        case MorphLFO::NOTE_MODE_TRIPLET:
          factor *= 2.0 / 3.0;
          break;
        case MorphLFO::NOTE_MODE_DOTTED:
          factor *= 3.0 / 2.0;
          break;
        default:
          ;
      }
      if (cfg->sync_voices)
        state.raw_phase = time_info.ppq_pos / factor;
      else
        state.raw_phase = state.ppq_count / factor;
    }
  const double old_phase = state.phase;
  state.phase = normalize_phase (state.raw_phase + cfg->start_phase / 360);
  constexpr double epsilon = 1 / 1000.;  // reliable comparision of floating point values
  if (state.phase + epsilon < old_phase)
    {
      // retrigger random lfo
      state.last_random_value = state.random_value;
      state.random_value = random_gen()->random_double_range (-1, 1);
    }

  if (cfg->wave_type == MorphLFO::WAVE_SINE)
    {
      state.value = sin (state.phase * M_PI * 2);
    }
  else if (cfg->wave_type == MorphLFO::WAVE_TRIANGLE)
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
  else if (cfg->wave_type == MorphLFO::WAVE_SAW_UP)
    {
      state.value = -1 + 2 * state.phase;
    }
  else if (cfg->wave_type == MorphLFO::WAVE_SAW_DOWN)
    {
      state.value = 1 - 2 * state.phase;
    }
  else if (cfg->wave_type == MorphLFO::WAVE_SQUARE)
    {
      if (state.phase < 0.5)
        state.value = -1;
      else
        state.value = 1;
    }
  else if (cfg->wave_type == MorphLFO::WAVE_RANDOM_SH)
    {
      state.value = state.random_value;
    }
  else if (cfg->wave_type == MorphLFO::WAVE_RANDOM_LINEAR)
    {
      state.value = state.last_random_value * (1 - state.phase) + state.random_value * state.phase;
    }
  else
    {
      g_assert_not_reached();
    }

  state.value = state.value * cfg->depth + cfg->center;
  state.value = CLAMP (state.value, -1.0, 1.0);
}

void
MorphLFOModule::update_shared_state (const TimeInfo& time_info)
{
  update_lfo_value (shared_state->global_lfo_state, time_info);
}
