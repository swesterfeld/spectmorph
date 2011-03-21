/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smmorphplanvoice.hh"
#include "smmorphoutputmodule.hh"
#include <assert.h>
#include <map>

using namespace SpectMorph;
using std::vector;
using std::string;
using std::sort;
using std::map;

#define N_CONTROL_INPUTS 2

MorphPlanVoice::MorphPlanVoice (MorphPlan *plan) :
  m_control_input (N_CONTROL_INPUTS),
  m_output (NULL)
{
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
  for (size_t i = 0; i < modules.size(); i++)
    {
      modules[i].module->set_config (modules[i].op);
    }
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

bool
MorphPlanVoice::try_update (MorphPlan *new_plan)
{
  vector<string>               old_ids, new_ids;
  map<string, MorphOperator *> op_map;

  printf ("try_update\n");

  // make a list of old operator ids
  for (size_t i = 0; i < modules.size(); i++)
    {
      string id = modules[i].op->id();
      if (id.empty())
        return false;
      old_ids.push_back (id);
    }
  // make a list of new operator ids
  const vector<MorphOperator *>& new_ops = new_plan->operators();
  for (vector<MorphOperator *>::const_iterator oi = new_ops.begin(); oi != new_ops.end(); oi++)
    {
      string id = (*oi)->id();
      if (id.empty())
        return false;
      new_ids.push_back (id);
      op_map[id] = *oi;
    }

  // update can only be done if the id lists match
  sort (old_ids.begin(), old_ids.end());
  sort (new_ids.begin(), new_ids.end());

  if (old_ids != new_ids)
    return false;

  // exchange old operators with new operators
  for (size_t i = 0; i < modules.size(); i++)
    {
      modules[i].op = op_map[modules[i].op->id()];
      assert (modules[i].op);
    }
  // reconfigure modules
  for (size_t i = 0; i < modules.size(); i++)
    {
      modules[i].module->set_config (modules[i].op);
    }
  printf (" - good update\n");
  return true;
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
