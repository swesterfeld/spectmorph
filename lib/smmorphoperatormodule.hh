// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_OPERATOR_MODULE_HH
#define SPECTMORPH_MORPH_OPERATOR_MODULE_HH

#include "smmorphoperator.hh"
#include "smlivedecodersource.hh"
#include "smrandom.hh"
#include "smtimeinfo.hh"
#include "smrtmemory.hh"

#include <string>

namespace SpectMorph
{

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
  float                               m_notify_value = 0;
  bool                                m_have_notify_value = false;

  Random *random_gen() const;
  RTMemoryArea *rt_memory_area() const;
  TimeInfo time_info() const;
  float apply_modulation (const ModulationData& mod_data) const;
  void set_notify_value (float value);
public:
  MorphOperatorModule (MorphPlanVoice *voice);
  virtual ~MorphOperatorModule();

  virtual void set_config (const MorphOperatorConfig *op_cfg) = 0;
  virtual LiveDecoderSource *source();
  virtual float value();
  virtual void reset_value (const TimeInfo& time_info);
  virtual void update_shared_state (const TimeInfo& time_info);
  virtual MorphModuleSharedState *create_shared_state();
  virtual void set_shared_state (MorphModuleSharedState *new_shared_state);

  void set_ptr_id (MorphOperator::PtrID ptr_id);
  bool get_notify_value (float& value);

  static MorphOperatorModule *create (const std::string& type, MorphPlanVoice *voice);
};

}

#endif
