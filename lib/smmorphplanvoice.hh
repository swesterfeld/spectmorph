// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_PLAN_VOICE_HH
#define SPECTMORPH_MORPH_PLAN_VOICE_HH

#include "smmorphplan.hh"
#include "smmorphoperatormodule.hh"
#include "smmorphplansynth.hh"
#include "smnotifybuffer.hh"

namespace SpectMorph {

class MorphOutputModule;
class MorphPlanSynth;

class MorphPlanVoice {
protected:
  std::vector<MorphPlanSynth::OpModule> modules;

  std::vector<double>           m_control_input;
  MorphOutputModule            *m_output = nullptr;
  float                         m_mix_freq;
  float                         m_velocity;
  MorphPlanSynth               *m_morph_plan_synth;

  void configure_modules();

public:
  MorphPlanVoice (float mix_freq, MorphPlanSynth *synth);
  ~MorphPlanVoice();

  void cheap_update (MorphPlanSynth::UpdateP update);
  void full_update (MorphPlanSynth::FullUpdateVoice& full_update_voice);

  MorphOperatorModule *module (const MorphOperatorPtr& ptr);

  double control_input (double value, MorphOperator::ControlType ctype, MorphOperatorModule *module);
  void   set_control_input (int i, double value);
  void   set_velocity (float velocity);

  float velocity() const;
  float mix_freq() const;

  MorphOutputModule *output();
  MorphPlanSynth *morph_plan_synth() const;

  void update_shared_state (const TimeInfo& time_info);
  void reset_value (const TimeInfo& time_info);
  void fill_notify_buffer (NotifyBuffer& notify_buffer);
};

}


#endif
