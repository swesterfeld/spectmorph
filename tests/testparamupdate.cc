// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphplan.hh"
#include "smmorphplanvoice.hh"
#include "smmorphplansynth.hh"
#include "smmorphoutputmodule.hh"
#include "smmain.hh"
#include "smproject.hh"
#include "smsynthinterface.hh"

using namespace SpectMorph;

static void
preinit_plan (MorphPlan& plan)
{
  MorphPlanSynth synth (44100, 1);

  auto update = synth.prepare_update (plan);
  synth.apply_update (update);
}

static void
measure_update (MorphPlan& plan, size_t n_voices)
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
  project.set_mix_freq (48000);

  Error error = project.load (argv[1]);
  assert (!error);

  MorphPlan *plan = project.morph_plan();

  fprintf (stderr, "SUCCESS: plan loaded, %zd operators found.\n", plan->operators().size());
  preinit_plan (*plan);
  measure_update (*plan, 1);
  measure_update (*plan, 64);
}
