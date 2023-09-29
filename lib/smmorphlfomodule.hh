// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_LFO_MODULE_HH
#define SPECTMORPH_MORPH_LFO_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smmorphlfo.hh"
#include "smwavset.hh"

namespace SpectMorph
{

class MorphLFOModule : public MorphOperatorModule
{
  const MorphLFO::Config *cfg = nullptr;

  struct LFOState
  {
    double phase              = 0;
    double raw_phase          = 0;
    double last_random_value  = 0;
    double random_value       = 0;
    double value              = 0;
    double ppq_count          = 0;
    double last_ppq_pos       = 0;
    double last_time_ms       = 0;
  } local_lfo_state;

  struct SharedState : public MorphModuleSharedState
  {
    bool     initialized = false;
    LFOState global_lfo_state;
  };
  SharedState *shared_state = nullptr;
  MorphModuleSharedState *create_shared_state() override;
  void set_shared_state (MorphModuleSharedState *new_shared_state) override;

  void update_lfo_value (LFOState& state, const TimeInfo& time_info);
  void restart_lfo (LFOState& state, const TimeInfo& time_info);
public:
  MorphLFOModule (MorphPlanVoice *voice);
  ~MorphLFOModule();

  void  set_config (const MorphOperatorConfig *cfg) override;
  float value() override;
  void  reset_value (const TimeInfo& time_info) override;
  void  update_shared_state (const TimeInfo& time_info) override;
};
}

#endif
