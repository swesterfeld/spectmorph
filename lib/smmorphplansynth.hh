// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_PLAN_SYNTH_HH
#define SPECTMORPH_MORPH_PLAN_SYNTH_HH

#include "smmorphplan.hh"
#include "smmorphoperator.hh"
#include "smrandom.hh"
#include "smtimeinfo.hh"
#include <map>
#include <memory>

namespace SpectMorph {

class MorphPlanVoice;
class MorphModuleSharedState;
class MorphOperatorModule;
class MorphOutputModule;

class MorphPlanSynth {
protected:
  std::vector<MorphPlanVoice *> voices;
  std::vector<std::unique_ptr<MorphModuleSharedState>> voices_shared_states;

  std::vector<std::string>                          m_last_update_ids;
  std::string                                       m_last_plan_id;
  std::vector<std::unique_ptr<MorphOperatorConfig>> m_active_configs;

  float           m_mix_freq;
  Random          m_random_gen;
  bool            m_have_cycle = false;

public:
  struct OpModule {
    std::unique_ptr<MorphOperatorModule> module;
    MorphOperator::PtrID ptr_id;
    MorphOperatorConfig *config = nullptr;
  };
  struct FullUpdateVoice
  {
    MorphOutputModule    *output_module = nullptr;
    std::vector<OpModule> new_modules;
  };
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
    std::vector<std::unique_ptr<MorphOperatorConfig>>    new_configs;
    std::vector<FullUpdateVoice>                         voice_full_updates;
    std::vector<std::unique_ptr<MorphModuleSharedState>> new_shared_states; // full updates only
  };
  typedef std::shared_ptr<Update> UpdateP;

  MorphPlanSynth (float mix_freq, size_t n_voices);
  ~MorphPlanSynth();

  UpdateP prepare_update (const MorphPlan& new_plan);
  void apply_update (UpdateP update);

  void update_shared_state (const TimeInfo& time_info);

  MorphPlanVoice *voice (size_t i) const;

  float   mix_freq() const;
  bool    have_output() const;
  Random *random_gen();
  bool    have_cycle() const;
};

}


#endif
