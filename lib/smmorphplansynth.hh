// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
  std::map<std::string, MorphModuleSharedState *> m_shared_state;

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
  bool  have_output() const;
};

}


#endif
