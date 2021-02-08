// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_SYNTH_HH
#define SPECTMORPH_MORPH_PLAN_SYNTH_HH

#include "smmorphplan.hh"
#include "smmorphoperator.hh"
#include "smrandom.hh"
#include <map>
#include <memory>

namespace SpectMorph {

class MorphPlanVoice;
class MorphModuleSharedState;
class TimeInfo;

class MorphPlanSynth {
protected:
  std::vector<MorphPlanVoice *> voices;
  std::map<std::string, MorphModuleSharedState *> m_shared_state;
  std::vector<std::string>                        m_last_update_ids;
  std::vector<MorphOperatorConfigP>               m_active_configs;

  float           m_mix_freq;
  Random          m_random_gen;

public:
  struct Update
  {
    struct Op
    {
      MorphOperator::PtrID ptr_id;
      std::string          type;
      MorphOperatorConfig *config = nullptr;
    };
    bool            cheap = false; // cheap update: same set of operators
    std::vector<Op> ops;
    std::vector<MorphOperatorConfigP> new_configs;
    std::vector<MorphOperatorConfigP> old_configs;
  };
  typedef std::shared_ptr<Update> UpdateP;

  MorphPlanSynth (float mix_freq);
  ~MorphPlanSynth();

  MorphPlanVoice *add_voice();
  void update_plan (MorphPlanPtr new_plan);
  UpdateP prepare_update (MorphPlanPtr new_plan);
  void apply_update (UpdateP update);

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
