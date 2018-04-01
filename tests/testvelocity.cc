// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smmath.hh"
#include "smutils.hh"

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  double vrange_db = (argc == 2) ? atof (argv[1]) : 30;

  for (int i = 0; i < 128; i++)
    {
      double v = i / 127.;

      sm_printf ("%f %f\n", v, velocity_to_gain (v, vrange_db));
    }
}
