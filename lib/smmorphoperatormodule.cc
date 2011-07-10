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

#include "smmorphoperatormodule.hh"

#include "smmorphlinearmodule.hh"
#include "smmorphoutputmodule.hh"
#include "smmorphsourcemodule.hh"
#include "smmorphlfomodule.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphOperatorModule::MorphOperatorModule (MorphPlanVoice *voice, size_t n_dependencies) :
  morph_plan_voice (voice),
  m_dependencies (n_dependencies)
{
}

MorphOperatorModule::~MorphOperatorModule()
{
  // virtual destructor to allow subclass deletion
}

static LeakDebugger shared_state_leak_debugger ("SpectMorph::MorphModuleSharedState");

MorphModuleSharedState::MorphModuleSharedState()
{
  shared_state_leak_debugger.add (this);
}

MorphModuleSharedState::~MorphModuleSharedState()
{
  shared_state_leak_debugger.del (this);
}

LiveDecoderSource *
MorphOperatorModule::source()
{
  return NULL;
}

float
MorphOperatorModule::value()
{
  return 0;
}

void
MorphOperatorModule::reset_value()
{
}

void
MorphOperatorModule::update_value (double time_ms)
{
}

void
MorphOperatorModule::update_shared_state (double time_ms)
{
}

float
MorphOperatorModule::latency_ms()
{
  return 0; // default
}

void
MorphOperatorModule::set_latency_ms (float latency_ms)
{
  // default
}

void
MorphOperatorModule::update_dependency (size_t i, MorphOperatorModule *dep_mod)
{
  g_return_if_fail (i < m_dependencies.size());

  m_dependencies[i] = dep_mod;
}

const vector<MorphOperatorModule *>&
MorphOperatorModule::dependencies() const
{
  return m_dependencies;
}

int&
MorphOperatorModule::update_value_tag()
{
  return m_update_value_tag;
}

MorphOperatorModule*
MorphOperatorModule::create (MorphOperator *op, MorphPlanVoice *voice)
{
  string type = op->type();

  if (type == "SpectMorph::MorphLinear") return new MorphLinearModule (voice);
  if (type == "SpectMorph::MorphSource") return new MorphSourceModule (voice);
  if (type == "SpectMorph::MorphOutput") return new MorphOutputModule (voice);
  if (type == "SpectMorph::MorphLFO")    return new MorphLFOModule (voice);

  return NULL;
}
