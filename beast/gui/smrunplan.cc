/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
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
#include "smmorphplan.hh"
#include "smmorphoutput.hh"
#include "smmorphoperatormodule.hh"
#include "smmain.hh"

#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::string;

namespace SpectMorph
{

class MorphLinearModule : public MorphOperatorModule
{
public:
  MorphLinearModule();

  void set_config (MorphOperator *op);
};

MorphLinearModule::MorphLinearModule()
{
}

void
MorphLinearModule::set_config (MorphOperator *op)
{
}

class MorphSourceModule : public MorphOperatorModule
{
public:
  MorphSourceModule();

  void set_config (MorphOperator *op);
};

MorphSourceModule::MorphSourceModule()
{
}

void
MorphSourceModule::set_config (MorphOperator *op)
{
}

class MorphOutputModule : public MorphOperatorModule
{
public:
  MorphOutputModule();

  void set_config (MorphOperator *op);
  void process (size_t n_values, float *values);
};

MorphOutputModule::MorphOutputModule()
{
}

void
MorphOutputModule::set_config (MorphOperator *op)
{
}

void
MorphOutputModule::process (size_t n_values, float *values)
{
  for (size_t i = 0; i < n_values; i++)
    values[i] = 0.0;
}

}

MorphOperatorModule*
MorphOperatorModule::create (MorphOperator *op)
{
  string type = op->type();

  if (type == "SpectMorph::MorphLinear") return new MorphLinearModule();
  if (type == "SpectMorph::MorphSource") return new MorphSourceModule();
  if (type == "SpectMorph::MorphOutput") return new MorphOutputModule();

  return NULL;
}

namespace SpectMorph {

class MorphPlanVoice {
  vector<MorphOperatorModule *> modules;

  MorphOutputModule            *m_output;
public:
  MorphPlanVoice (MorphPlan *plan)
  {
    const vector<MorphOperator *>& ops = plan->operators();
    for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
      {
        MorphOperatorModule *module = MorphOperatorModule::create (*oi);
        string type = (*oi)->type();

        if (!module)
          {
            g_warning ("operator type %s lacks MorphOperatorModule\n", type.c_str());
          }
        else
          {
            modules.push_back (module);

            if (type == "SpectMorph::MorphOutput")
              m_output = dynamic_cast<MorphOutputModule *> (module);
          }
      }
  }
  MorphOutputModule *
  output()
  {
    return m_output;
  }
};

}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);
  if (argc != 2)
    {
      printf ("usage: %s <plan>\n", argv[0]);
      exit (1);
    }

  MorphPlan plan;
  GenericIn *in = StdioIn::open (argv[1]);
  if (!in)
    {
      g_printerr ("Error opening '%s'.\n", argv[1]);
      exit (1);
    }
  plan.load (in);
  printf ("\n\nSUCCESS: plan loaded, %zd operators found.\n", plan.operators().size());

  MorphPlanVoice voice (&plan);
  assert (voice.output());

  vector<float> samples;

  for (size_t i = 0; i < 44100; i++)
    {
      float f;

      voice.output()->process (1, &f);
      samples.push_back (f);
    }
}
