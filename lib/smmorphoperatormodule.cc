// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
MorphOperatorModule::update_shared_state (const TimeInfo& time_info)
{
}

void
MorphOperatorModule::clear_dependencies()
{
  m_dependencies.clear();
}

void
MorphOperatorModule::add_dependency (MorphOperatorModule *dep_mod)
{
  if (dep_mod)
    m_dependencies.push_back (dep_mod);
}

const vector<MorphOperatorModule *>&
MorphOperatorModule::dependencies() const
{
  return m_dependencies;
}

int&
MorphOperatorModule::update_value_tag()
{
  return m_update_value_tag;
}

Random *
MorphOperatorModule::random_gen() const
{
  return morph_plan_voice->morph_plan_synth()->random_gen();
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
MorphOperatorModule::apply_modulation (float base, const ModulationData& mod_data) const
{
  double value = 0;
  for (const auto& entry : mod_data.entries)
    {
      double mod_value = 0;

      if (entry.control_type == MorphOperator::CONTROL_OP)
        mod_value = morph_plan_voice->module (entry.control_op)->value();
      else
        mod_value = morph_plan_voice->control_input (/* gui (not used) */ 0, entry.control_type, /* mod (not used) */ nullptr);

      value += mod_value * entry.mod_amount;
    }
  switch (mod_data.property_scale)
    {
      case Property::Scale::LOG:
        value = sm_clamp (base * exp2f (mod_data.value_scale * value), mod_data.min_value, mod_data.max_value);
        break;
      default:
        value = sm_clamp<float> (base + mod_data.value_scale * value, mod_data.min_value, mod_data.max_value);
    }
  return value;
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
