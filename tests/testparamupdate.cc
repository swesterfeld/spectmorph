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
#include "smmorphplanvoice.hh"
#include "smmorphplansynth.hh"
#include "smmorphoutputmodule.hh"
#include "smmain.hh"

#include <sys/time.h>

using namespace SpectMorph;

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static void
measure_update (MorphPlanPtr plan, size_t n_voices)
{
  MorphPlanSynth synth (44100);
  for (size_t i = 0; i < n_voices; i++)
    synth.add_voice();

  size_t runs = 1000000 / n_voices;

  double start = gettime();
  for (size_t j = 0; j < runs; j++)
    synth.update_plan (plan);
  double end = gettime();

  printf ("update (%zd voices): %f updates per ms\n", n_voices, 1 / ((end - start) * 1000 / runs));
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

  MorphPlanPtr plan = new MorphPlan();
  GenericIn *in = StdioIn::open (argv[1]);
  if (!in)
    {
      g_printerr ("Error opening '%s'.\n", argv[1]);
      exit (1);
    }
  plan->load (in);
  delete in;

  fprintf (stderr, "SUCCESS: plan loaded, %zd operators found.\n", plan->operators().size());

  measure_update (plan, 1);
  measure_update (plan, 10);
}
