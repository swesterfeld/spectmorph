// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplan.hh"
#include "smmorphplanvoice.hh"
#include "smmorphplansynth.hh"
#include "smmorphoutputmodule.hh"
#include "smmain.hh"
#include "smproject.hh"
#include "smsynthinterface.hh"

using namespace SpectMorph;

static void
preinit_plan (MorphPlanPtr plan)
{
  MorphPlanSynth synth (44100, 1);

  auto update = synth.prepare_update (plan);
  synth.apply_update (update);
}

static void
measure_update (MorphPlanPtr plan, size_t n_voices)
{
  MorphPlanSynth synth (44100, n_voices);

  size_t runs = 1000000 / n_voices;
  double start = get_time();
  for (size_t j = 0; j < runs; j++)
    synth.apply_update (synth.prepare_update (plan));
  double end = get_time();

  printf ("update (%zd voices): %f updates per ms\n", n_voices, 1 / ((end - start) * 1000 / runs));
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);
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
