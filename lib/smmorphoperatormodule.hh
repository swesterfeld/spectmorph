// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_OPERATOR_MODULE_HH
#define SPECTMORPH_MORPH_OPERATOR_MODULE_HH

#include "smmorphoperator.hh"
#include "smlivedecodersource.hh"
#include "smrandom.hh"

#include <string>

namespace SpectMorph
{

class TimeInfo
{
public:
  double time_ms = 0;
  double ppq_pos = 0;
};

class MorphPlanVoice;

class MorphModuleSharedState
{
public:
  MorphModuleSharedState();
  virtual ~MorphModuleSharedState();
};

class MorphOperatorModule
{
protected:
  MorphPlanVoice                     *morph_plan_voice;
  MorphOperator::PtrID                m_ptr_id;

  Random *random_gen() const;
  TimeInfo time_info() const;
  float apply_modulation (const ModulationData& mod_data) const;
public:
  MorphOperatorModule (MorphPlanVoice *voice);
  virtual ~MorphOperatorModule();

  virtual void set_config (const MorphOperatorConfig *op_cfg) = 0;
  virtual LiveDecoderSource *source();
  virtual float value();
  virtual void reset_value (const TimeInfo& time_info);
  virtual void update_shared_state (const TimeInfo& time_info);

  void set_ptr_id (MorphOperator::PtrID ptr_id);

  static MorphOperatorModule *create (const std::string& type, MorphPlanVoice *voice);
};

}

#endif
