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
#include "smmain.hh"

using namespace SpectMorph;

using std::vector;

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

  const vector<MorphOperator *>& ops = plan.operators();

  MorphOutput *output = NULL;
  for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
    {
      MorphOperator *op = *oi;
      if (!strcmp (op->type(), "SpectMorph::MorphOutput"))
        output = dynamic_cast<MorphOutput *> (op);
    }
  if (!output)
    {
      printf ("failed to find output\n");
      exit (1);
    }
  printf ("Output: %s\n", output->name().c_str());
  MorphOperator *out_op = output->channel_op (0);
  if (!out_op)
    {
      printf ("no output 0 defined\n");
      exit (1);
    }
  printf ("Output[0] = %s\n", out_op->name().c_str());
}
