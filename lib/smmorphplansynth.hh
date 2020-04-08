// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_SYNTH_HH
#define SPECTMORPH_MORPH_PLAN_SYNTH_HH

#include "smmorphplan.hh"
#include "smmorphoperator.hh"
#include "smrandom.hh"
#include <map>

namespace SpectMorph {

class MorphPlanVoice;
class MorphModuleSharedState;
class TimeInfo;

class MorphPlanSynth {
protected:
  std::vector<MorphPlanVoice *> voices;
  std::map<std::string, MorphModuleSharedState *> m_shared_state;

  float        m_mix_freq;
  MorphPlanPtr plan;
  Random       m_random_gen;

public:
  MorphPlanSynth (float mix_freq);
  ~MorphPlanSynth();

  MorphPlanVoice *add_voice();
  void update_plan (MorphPlanPtr new_plan);

  MorphModuleSharedState *shared_state (MorphOperator *op);
  void set_shared_state (MorphOperator *op, MorphModuleSharedState *shared_state);

  void update_shared_state (const TimeInfo& time_info);
  void free_shared_state();

  float   mix_freq() const;
  bool    have_output() const;
  Random *random_gen();
};

}


#endif
