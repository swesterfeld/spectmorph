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

#ifndef SPECTMORPH_MORPH_PLAN_VOICE_HH
#define SPECTMORPH_MORPH_PLAN_VOICE_HH

#include "smmorphplan.hh"
#include "smmorphoperatormodule.hh"

namespace SpectMorph {

class MorphOutputModule;
class MorphPlanSynth;

class MorphPlanVoice {
protected:
  struct OpModule {
    MorphOperatorModule *module;
    MorphOperator       *op;
  };
  std::vector<OpModule> modules;

  std::vector<double>           m_control_input;
  MorphOutputModule            *m_output;
  MorphPlanPtr                  m_plan;
  float                         m_mix_freq;
  MorphPlanSynth               *m_morph_plan_synth;

  bool try_update (MorphPlanPtr plan);
  void clear_modules();
  void create_modules();
  void configure_modules();

public:
  MorphPlanVoice (MorphPlanPtr plan, float mix_freq, MorphPlanSynth *synth = NULL);
  ~MorphPlanVoice();

  void update (MorphPlanPtr plan);

  MorphOperatorModule *module (MorphOperator *op);

  double control_input (int i);
  void   set_control_input (int i, double value);

  float mix_freq() const;

  MorphOutputModule *output();
  MorphPlanSynth *morph_plan_synth() const;

  void update_shared_state (double time_ms);
};

}


#endif
