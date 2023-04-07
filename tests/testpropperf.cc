// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmidisynth.hh"
#include "smmain.hh"
#include "smproject.hh"
#include "smsynthinterface.hh"
#include "smmorphoutput.hh"

using namespace SpectMorph;

using std::vector;

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);
  if (argc != 2)
    {
      fprintf (stderr, "usage: testpropperf <plan>\n");
      return 1;
    }

  Project project;
  project.set_mix_freq (48000);

  Error error = project.load (argv[1]);
  assert (!error);

  MorphPlan *plan = project.morph_plan();
  auto ops = plan->operators();
  for (MorphOperator *op : ops)
    {
      if (op->type_name() == "Output")
        {
          MorphOutput *out = dynamic_cast<MorphOutput *> (op);
          Property *prop = out->property (MorphOutput::P_VELOCITY_SENSITIVITY);
          double start = get_time();
          int runs = 20000;
          for (int i = 0; i < runs / 2; i++)
            {
              prop->set_float (1);
              project.try_update_synth();
              prop->set_float (2);
              project.try_update_synth();
            }
          double end = get_time();
          printf ("%f updates per ms\n", 1 / ((end - start) * 1000 / runs));
        }
    }
}
