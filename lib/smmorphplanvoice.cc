// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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

#define N_CONTROL_INPUTS 2

MorphPlanVoice::MorphPlanVoice (float mix_freq, MorphPlanSynth *synth) :
  m_control_input (N_CONTROL_INPUTS),
  m_output (NULL),
  m_mix_freq (mix_freq),
  m_morph_plan_synth (synth)
{
  leak_debugger.add (this);
}

void
MorphPlanVoice::configure_modules()
{
  float latency_ms = 0;
  for (size_t i = 0; i < modules.size(); i++)
    {
      modules[i].module->set_config (modules[i].op);
      latency_ms = max (modules[i].module->latency_ms(), latency_ms);
    }
  for (size_t i = 0; i < modules.size(); i++)
    {
      modules[i].module->set_latency_ms (latency_ms);
    }
}

void
MorphPlanVoice::create_modules (MorphPlanPtr plan)
{
  if (!plan)
    return;

  const vector<MorphOperator *>& ops = plan->operators();
  for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
    {
      MorphOperatorModule *module = MorphOperatorModule::create (*oi, this);
      string type = (*oi)->type();

      if (!module)
        {
          g_warning ("operator type %s lacks MorphOperatorModule\n", type.c_str());
        }
      else
        {
          OpModule op_module;

          op_module.module = module;
          op_module.op     = (*oi);

          modules.push_back (op_module);

          if (type == "SpectMorph::MorphOutput")
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
MorphPlanVoice::module (MorphOperator *op)
{
  for (size_t i = 0; i < modules.size(); i++)
    if (modules[i].op == op)
      return modules[i].module;

  return NULL;
}

void
MorphPlanVoice::full_update (MorphPlanPtr plan)
{
  /* This will loose the original state information which means the audio
   * will not transition smoothely. However, this should only occur for plan
   * changes, not parameter updates.
   */
  clear_modules();
  create_modules (plan);
  configure_modules();
}

void
MorphPlanVoice::cheap_update (map<string, MorphOperator *>& op_map)
{
  // exchange old operators with new operators
  for (size_t i = 0; i < modules.size(); i++)
    {
      modules[i].op = op_map[modules[i].op->id()];
      assert (modules[i].op);
    }
  // reconfigure modules
  configure_modules();
}

double
MorphPlanVoice::control_input (int i)
{
  assert (i >= 0 && i < N_CONTROL_INPUTS);
  return m_control_input[i];
}

void
MorphPlanVoice::set_control_input (int i, double value)
{
  assert (i >= 0 && i < N_CONTROL_INPUTS);

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
MorphPlanVoice::update_shared_state (double time_ms)
{
  for (size_t i = 0; i < modules.size(); i++)
    modules[i].module->update_shared_state (time_ms);
}
