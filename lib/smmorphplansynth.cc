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

#include "smmorphplansynth.hh"
#include "smmorphplanvoice.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

using std::map;

static LeakDebugger leak_debugger ("SpectMorph::MorphPlanSynth");

MorphPlanSynth::MorphPlanSynth (float mix_freq) :
  m_mix_freq (mix_freq)
{
  leak_debugger.add (this);
}

MorphPlanSynth::~MorphPlanSynth()
{
  leak_debugger.del (this);

  free_shared_state();

  for (size_t i = 0; i < voices.size(); i++)
    delete voices[i];

  voices.clear();
}

MorphPlanVoice *
MorphPlanSynth::add_voice()
{
  MorphPlanVoice *voice = new MorphPlanVoice (NULL, m_mix_freq, this);
  voices.push_back (voice);
  return voice;
}

void
MorphPlanSynth::update_plan (MorphPlanPtr new_plan)
{
  free_shared_state();

  for (size_t i = 0; i < voices.size(); i++)
    voices[i]->update (new_plan);
}

void
MorphPlanSynth::update_shared_state (double time_ms)
{
  if (voices.empty())
    return;
  voices[0]->update_shared_state (time_ms);
}

MorphModuleSharedState *
MorphPlanSynth::shared_state (MorphOperator *op)
{
  return m_shared_state[op];
}

void
MorphPlanSynth::set_shared_state (MorphOperator *op, MorphModuleSharedState *shared_state)
{
  m_shared_state[op] = shared_state;
}

float
MorphPlanSynth::mix_freq() const
{
  return m_mix_freq;
}

void
MorphPlanSynth::free_shared_state()
{
  map<MorphOperator *, MorphModuleSharedState *>::iterator si;
  for (si = m_shared_state.begin(); si != m_shared_state.end(); si++)
    delete si->second;
  m_shared_state.clear();
}
