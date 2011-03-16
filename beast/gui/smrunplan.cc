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
#include "smmorphplanvoice.hh"
#include "smmorphoutputmodule.hh"
#include "smmain.hh"

#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::string;

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
  fprintf (stderr, "SUCCESS: plan loaded, %zd operators found.\n", plan.operators().size());

  MorphPlanVoice voice (&plan);
  assert (voice.output());

  vector<float> samples (44100);
  voice.output()->retrigger (0, 440, 100, 44100);
  voice.output()->process (samples.size(), &samples[0]);
  for (size_t i = 0; i < samples.size(); i++)
    {
      printf ("%.17g\n", samples[i]);
    }
}
