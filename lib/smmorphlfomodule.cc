/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smmorphlfomodule.hh"
#include "smmorphlfo.hh"
#include "smmorphplan.hh"
#include "smwavsetrepo.hh"
#include "smleakdebugger.hh"
#include "smmorphplanvoice.hh"
#include "smmorphplansynth.hh"
#include "smmath.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::MorphLFOModule");

MorphLFOModule::MorphLFOModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice, 0)
{
  leak_debugger.add (this);

  phase = 0;
  shared_state = NULL;
}

MorphLFOModule::~MorphLFOModule()
{
  leak_debugger.del (this);
}

void
MorphLFOModule::set_config (MorphOperator *op)
{
  MorphLFO *lfo = dynamic_cast<MorphLFO *> (op);

  frequency = lfo->frequency();
  depth = lfo->depth();
  center = lfo->center();
  start_phase = lfo->start_phase();
  sync_voices = lfo->sync_voices();
  wave_type = lfo->wave_type();

  MorphPlanSynth *synth = morph_plan_voice->morph_plan_synth();
  if (synth)
    {
      shared_state = dynamic_cast<SharedState *> (synth->shared_state (op));
      if (!shared_state)
        {
          shared_state = new SharedState();
          shared_state->phase = start_phase / 360;
          shared_state->value = compute_value (shared_state->phase);
          update_shared_state (0);
        }
    }
}

float
MorphLFOModule::value()
{
  return sync_voices ? shared_state->value : m_value;
}

void
MorphLFOModule::reset_value()
{
  phase = start_phase / 360;
}

double
MorphLFOModule::compute_value (double phase)
{
  double value;

  if (wave_type == MorphLFO::WAVE_SINE)
    {
      value = sin (phase * M_PI * 2);
    }
  else if (wave_type == MorphLFO::WAVE_TRIANGLE)
    {
      double phase_mod = fmod (phase + 1, 1);
      if (phase_mod < 0.25)
        {
          value = 4 * phase_mod;
        }
      else if (phase_mod < 0.75)
        {
          value = (phase_mod - 0.5) * -4;
        }
      else
        {
          value = 4 * (phase_mod - 1);
        }
    }
  else
    {
      g_assert_not_reached();
    }

  value = value * depth + center;
  return CLAMP (value, -1.0, 1.0);
}

void
MorphLFOModule::update_value (double time_ms)
{
  phase += time_ms / 1000 * frequency;

  m_value = compute_value (phase);
}

void
MorphLFOModule::update_shared_state (double time_ms)
{
  shared_state->phase += time_ms / 1000 * frequency;
  shared_state->value = compute_value (shared_state->phase);
}
