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

using namespace SpectMorph;
using std::vector;
using std::string;

MorphPlanVoice::MorphPlanVoice (MorphPlan *plan)
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
