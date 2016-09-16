// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
  float                         m_mix_freq;
  MorphPlanSynth               *m_morph_plan_synth;

  void clear_modules();
  void create_modules (MorphPlanPtr plan);
  void configure_modules();

public:
  MorphPlanVoice (float mix_freq, MorphPlanSynth *synth);
  ~MorphPlanVoice();

  void cheap_update (std::map<std::string, MorphOperator *>& op_map);
  void full_update (MorphPlanPtr plan);

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
