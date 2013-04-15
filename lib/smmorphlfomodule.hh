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

  double  phase;
  double  m_value;

  struct SharedState : public MorphModuleSharedState
  {
    double  phase;
    double  value;
  };
  SharedState *shared_state;

  double compute_value (double phase);
public:
  MorphLFOModule (MorphPlanVoice *voice);
  ~MorphLFOModule();

  void  set_config (MorphOperator *op);
  float value();
  void  reset_value();
  void  update_value (double time_ms);
  void  update_shared_state (double time_ms);
};
}

#endif
