// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmorphoperator.hh"
#include "smlivedecodersource.hh"
#include "smrandom.hh"
#include "smtimeinfo.hh"
#include "smrtmemory.hh"

#include <array>
#include <string>

namespace SpectMorph
{

class MorphPlanVoice;

class MorphModuleSharedState
{
  LeakDebugger2 leak_debugger2 { "SpectMorph::MorphModuleSharedState" };
public:
  virtual ~MorphModuleSharedState();
};

class MorphOperatorModule
{
public:
  static constexpr uint               MAX_NOTIFY_VALUES = 2;
protected:
  MorphPlanVoice                     *morph_plan_voice;
  MorphOperator::PtrID                m_ptr_id;
  std::array<float,MAX_NOTIFY_VALUES> m_notify_values {};
  uint                                m_have_notify_values = 0;

  Random *random_gen() const;
  RTMemoryArea *rt_memory_area() const;
  TimeInfo time_info() const;
  float apply_modulation (const ModulationData& mod_data) const;
  void set_notify_value (uint pos, float value);
public:
  MorphOperatorModule (MorphPlanVoice *voice);
  virtual ~MorphOperatorModule();

  virtual void set_config (const MorphOperatorConfig *op_cfg) = 0;
  virtual LiveDecoderSource *source();
  virtual float value();
  virtual void note_on (const TimeInfo& time_info);
  virtual void note_off();
  virtual void update_shared_state (const TimeInfo& time_info);
  virtual MorphModuleSharedState *create_shared_state();
  virtual void set_shared_state (MorphModuleSharedState *new_shared_state);

  void set_ptr_id (MorphOperator::PtrID ptr_id);

  const std::array<float, MAX_NOTIFY_VALUES>& get_notify_values (uint& count) const;

  static MorphOperatorModule *create (const std::string& type, MorphPlanVoice *voice);
};

}
