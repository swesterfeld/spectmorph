// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
preinit_plan (MorphPlanPtr plan)
{
  MorphPlanSynth synth (44100);
  synth.add_voice();
  synth.update_plan (plan);
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

  Project      project;
  MorphPlanPtr plan = new MorphPlan (project);
  GenericIn *in = StdioIn::open (argv[1]);
  if (!in)
    {
      g_printerr ("Error opening '%s'.\n", argv[1]);
      exit (1);
    }
  plan->load (in);
  delete in;

  fprintf (stderr, "SUCCESS: plan loaded, %zd operators found.\n", plan->operators().size());
  preinit_plan (plan);
  measure_update (plan, 1);
  measure_update (plan, 10);
}
