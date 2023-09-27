// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphoperatormodule.hh"

#include "smmorphlinearmodule.hh"
#include "smmorphgridmodule.hh"
#include "smmorphoutputmodule.hh"
#include "smmorphsourcemodule.hh"
#include "smmorphwavsourcemodule.hh"
#include "smmorphlfomodule.hh"
#include "smmorphplansynth.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphOperatorModule::MorphOperatorModule (MorphPlanVoice *voice) :
  morph_plan_voice (voice)
{
}

MorphOperatorModule::~MorphOperatorModule()
{
  // virtual destructor to allow subclass deletion
}

static LeakDebugger shared_state_leak_debugger ("SpectMorph::MorphModuleSharedState");

MorphModuleSharedState::MorphModuleSharedState()
{
  shared_state_leak_debugger.add (this);
}

MorphModuleSharedState::~MorphModuleSharedState()
{
  shared_state_leak_debugger.del (this);
}

LiveDecoderSource *
MorphOperatorModule::source()
{
  return NULL;
}

float
MorphOperatorModule::value()
{
  return 0;
}

void
MorphOperatorModule::reset_value (const TimeInfo& time_info)
{
}

void
MorphOperatorModule::set_notify_value (float value)
{
  m_notify_value = value;
  m_have_notify_value = true;
}

bool
MorphOperatorModule::get_notify_value (float& value)
{
  if (m_have_notify_value)
    {
      value = m_notify_value;
      return true;
    }
  else
    {
      return false;
    }
}

void
MorphOperatorModule::update_shared_state (const TimeInfo& time_info)
{
}

Random *
MorphOperatorModule::random_gen() const
{
  return morph_plan_voice->morph_plan_synth()->random_gen();
}

RTMemoryArea *
MorphOperatorModule::rt_memory_area() const
{
  MorphOutputModule *output = morph_plan_voice->output();
  return output->rt_memory_area();
}

TimeInfo
MorphOperatorModule::time_info() const
{
  MorphOutputModule *output = morph_plan_voice->output();
  TimeInfo time;

  if (output)
    time = output->compute_time_info();

  return time;
}

void
MorphOperatorModule::set_ptr_id (MorphOperator::PtrID ptr_id)
{
  m_ptr_id = ptr_id;
}

float
MorphOperatorModule::apply_modulation (const ModulationData& mod_data) const
{
  double base;
  double value = 0;

  /* main value */
  if (mod_data.main_control_type == MorphOperator::CONTROL_GUI)
    {
      base = mod_data.value;
    }
  else
    {
      /* to bind main value to operator or control signal, we
       *  - perform unimodular modulation with range [0:1]
       *  - set base value to minimum
       */
      base = mod_data.min_value;

      if (mod_data.main_control_type == MorphOperator::CONTROL_OP)
        {
          value = (morph_plan_voice->module (mod_data.main_control_op)->value() + 1) * 0.5;
        }
      else
        {
          value = (morph_plan_voice->control_input (/* gui */ 0, mod_data.main_control_type, /* mod (not used) */ nullptr) + 1) * 0.5;
        }
    }

  /* modulate main value */
  for (const auto& entry : mod_data.entries)
    {
      double mod_value = 0;

      if (entry.control_type == MorphOperator::CONTROL_OP)
        mod_value = morph_plan_voice->module (entry.control_op)->value();
      else
        mod_value = morph_plan_voice->control_input (/* gui (not used) */ 0, entry.control_type, /* mod (not used) */ nullptr);

      /* unipolar modulation: mod_value range [0..1]
       *  bipolar modulation: mod_value range [-1..1]
       */
      if (!entry.bipolar)
        mod_value = 0.5 * (mod_value + 1);

      value += mod_value * entry.amount;
    }
  switch (mod_data.property_scale)
    {
      case Property::Scale::LOG:
        value = base * exp2f (mod_data.value_scale * value);
        break;
      default:
        value = base + mod_data.value_scale * value;
    }
  return sm_clamp<float> (value, mod_data.min_value, mod_data.max_value);
}

MorphOperatorModule*
MorphOperatorModule::create (const std::string& type, MorphPlanVoice *voice)
{
  if (type == "SpectMorph::MorphLinear")    return new MorphLinearModule (voice);
  if (type == "SpectMorph::MorphGrid")      return new MorphGridModule (voice);
  if (type == "SpectMorph::MorphSource")    return new MorphSourceModule (voice);
  if (type == "SpectMorph::MorphWavSource") return new MorphWavSourceModule (voice);
  if (type == "SpectMorph::MorphOutput")    return new MorphOutputModule (voice);
  if (type == "SpectMorph::MorphLFO")       return new MorphLFOModule (voice);

  return NULL;
}
