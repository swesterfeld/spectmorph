// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphplanvoice.hh"
#include "smmorphoutputmodule.hh"
#include "smleakdebugger.hh"
#include <assert.h>
#include <map>

using namespace SpectMorph;
using std::vector;
using std::string;
using std::sort;
using std::map;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::MorphPlanVoice");

MorphPlanVoice::MorphPlanVoice (float mix_freq, MorphPlanSynth *synth) :
  m_control_input (MorphPlan::N_CONTROL_INPUTS),
  m_output (NULL),
  m_mix_freq (mix_freq),
  m_morph_plan_synth (synth)
{
  leak_debugger.add (this);
}

void
MorphPlanVoice::configure_modules()
{
  for (size_t i = 0; i < modules.size(); i++)
    modules[i].module->set_config (modules[i].config);
}

void
MorphPlanVoice::create_modules (MorphPlanSynth::UpdateP update)
{
  for (auto& op : update->ops)
    {
      MorphOperatorModule *module = MorphOperatorModule::create (op.type, this);

      if (!module)
        {
          g_warning ("operator type %s lacks MorphOperatorModule\n", op.type.c_str());
        }
      else
        {
          module->set_ptr_id (op.ptr_id);

          OpModule op_module;

          op_module.module = module;
          op_module.ptr_id = op.ptr_id;
          op_module.config = op.config;

          modules.push_back (op_module);

          if (op.type == "SpectMorph::MorphOutput")
            m_output = dynamic_cast<MorphOutputModule *> (module);
        }
    }
}

void
MorphPlanVoice::clear_modules()
{
  for (size_t i = 0; i < modules.size(); i++)
    {
      assert (modules[i].module != NULL);
      delete modules[i].module;
    }
  modules.clear();

  m_output = NULL;
}

MorphPlanVoice::~MorphPlanVoice()
{
  clear_modules();
  leak_debugger.del (this);
}

MorphOutputModule *
MorphPlanVoice::output()
{
  return m_output;
}

MorphOperatorModule *
MorphPlanVoice::module (const MorphOperatorPtr& ptr)
{
  MorphOperator::PtrID ptr_id = ptr.ptr_id();

  for (size_t i = 0; i < modules.size(); i++)
    if (modules[i].ptr_id == ptr_id)
      return modules[i].module;

  return NULL;
}

void
MorphPlanVoice::full_update (MorphPlanSynth::UpdateP update)
{
  /* This will loose the original state information which means the audio
   * will not transition smoothely. However, this should only occur for plan
   * changes, not parameter updates.
   */
  clear_modules();
  create_modules (update);
  configure_modules();
}

void
MorphPlanVoice::cheap_update (MorphPlanSynth::UpdateP update)
{
  g_return_if_fail (update->ops.size() == modules.size());

  // exchange old operators with new operators
  for (size_t i = 0; i < modules.size(); i++)
    {
      assert (modules[i].ptr_id == update->ops[i].ptr_id);
      modules[i].config = update->ops[i].config;
      assert (modules[i].config);
    }

  // reconfigure modules
  configure_modules();
}

double
MorphPlanVoice::control_input (double value, MorphOperator::ControlType ctype, MorphOperatorModule *module)
{
  switch (ctype)
    {
      case MorphOperator::CONTROL_GUI:      return value;
      case MorphOperator::CONTROL_SIGNAL_1: return m_control_input[0];
      case MorphOperator::CONTROL_SIGNAL_2: return m_control_input[1];
      case MorphOperator::CONTROL_SIGNAL_3: return m_control_input[2];
      case MorphOperator::CONTROL_SIGNAL_4: return m_control_input[3];
      case MorphOperator::CONTROL_OP:       return module->value();
      default:                              g_assert_not_reached();
    }
}

void
MorphPlanVoice::set_control_input (int i, double value)
{
  assert (i >= 0 && i < MorphPlan::N_CONTROL_INPUTS);

  m_control_input[i] = value;
}

float
MorphPlanVoice::mix_freq() const
{
  return m_mix_freq;
}

MorphPlanSynth *
MorphPlanVoice::morph_plan_synth() const
{
  return m_morph_plan_synth;
}

void
MorphPlanVoice::update_shared_state (const TimeInfo& time_info)
{
  for (size_t i = 0; i < modules.size(); i++)
    modules[i].module->update_shared_state (time_info);
}

void
MorphPlanVoice::reset_value (const TimeInfo& time_info)
{
  for (size_t i = 0; i < modules.size(); i++)
    modules[i].module->reset_value (time_info);
}
