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

#ifndef SPECTMORPH_MORPH_PLAN_SYNTH_HH
#define SPECTMORPH_MORPH_PLAN_SYNTH_HH

#include "smmorphplan.hh"
#include "smmorphoperator.hh"
#include <map>

namespace SpectMorph {

class MorphPlanVoice;
class MorphModuleSharedState;

class MorphPlanSynth {
protected:
  std::vector<MorphPlanVoice *> voices;
  std::map<MorphOperator *, MorphModuleSharedState *> m_shared_state;

  float        m_mix_freq;
  MorphPlanPtr plan;

public:
  MorphPlanSynth (float mix_freq);
  ~MorphPlanSynth();

  MorphPlanVoice *add_voice();
  void update_plan (MorphPlanPtr new_plan);

  MorphModuleSharedState *shared_state (MorphOperator *op);
  void set_shared_state (MorphOperator *op, MorphModuleSharedState *shared_state);

  void update_shared_state (double time_ms);
  void free_shared_state();

  float mix_freq() const;
};

}


#endif
