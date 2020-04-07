// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LFO_MODULE_HH
#define SPECTMORPH_MORPH_LFO_MODULE_HH

#include "smmorphoperatormodule.hh"
#include "smmorphlfo.hh"
#include "smwavset.hh"

namespace SpectMorph
{

class MorphLFOModule : public MorphOperatorModule
{
  MorphLFO::WaveType wave_type;

  float   frequency;
  float   depth;
  float   center;
  float   start_phase;
  bool    sync_voices;

  struct LFOState
  {
    double phase              = 0;
    double last_random_value  = 0;
    double random_value       = 0;
    double value              = 0;
  } local_lfo_state;

  double last_time_ms = 0;

  struct SharedState : public MorphModuleSharedState
  {
    double   time_ms = 0;
    LFOState global_lfo_state;
  };
  SharedState *shared_state;

  void update_lfo_value (LFOState& state, double time_ms);
  void restart_lfo (LFOState& state);
public:
  MorphLFOModule (MorphPlanVoice *voice);
  ~MorphLFOModule();

  void  set_config (MorphOperator *op) override;
  float value() override;
  void  reset_value (const TimeInfo& time_info) override;
  void  update_shared_state (double time_ms) override;
};
}

#endif
