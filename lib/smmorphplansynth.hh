// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
  std::map<MorphOperator::PtrID, MorphModuleSharedState *> m_shared_state;
  std::vector<std::string>                        m_last_update_ids;
  std::string                                     m_last_plan_id;
  std::vector<MorphOperatorConfigP>               m_active_configs;

  float           m_mix_freq;
  Random          m_random_gen;
  bool            m_have_cycle = false;

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
    bool            have_cycle = false; // plan contains cycles?
    std::vector<Op> ops;
    std::vector<MorphOperatorConfigP> new_configs;
    std::vector<MorphOperatorConfigP> old_configs;

    bool is_full_update() { return !cheap; }
  };
  typedef std::shared_ptr<Update> UpdateP;

  MorphPlanSynth (float mix_freq, size_t n_voices);
  ~MorphPlanSynth();

  UpdateP prepare_update (const MorphPlan& new_plan);
  void apply_update (UpdateP update);

  MorphModuleSharedState *shared_state (MorphOperator::PtrID ptr_id);
  void set_shared_state (MorphOperator::PtrID ptr_id, MorphModuleSharedState *shared_state);

  void update_shared_state (const TimeInfo& time_info);
  void free_shared_state();

  MorphPlanVoice *voice (size_t i) const;

  float   mix_freq() const;
  bool    have_output() const;
  Random *random_gen();
  bool    have_cycle() const;
};

}


#endif
