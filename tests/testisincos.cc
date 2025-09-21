// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmath.hh"
#include "smmain.hh"
#include "smutils.hh"

#include <stdio.h>
#include <assert.h>

#include <algorithm>
#include <string>

using namespace SpectMorph;
using std::max;
using std::string;

static float acc[2] = { 0, 0 };

void
perf_test()
{
  double clocks_per_sec = 2500.0 * 1000 * 1000;
  double start = get_time();
  static int CALLS = 1000 * 1000 * 1000;
  for (int i = 0; i < CALLS; i++)
    {
      const float is = int_sinf (i);
      const float ic = int_cosf (i);

      acc[0] += is;
      acc[1] += ic;
    }
  double end = get_time();
  printf ("int_sincos: %f clocks/invocation\n", clocks_per_sec * (end - start) / CALLS);
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  if (argc == 2 && string (argv[1]) == "perf")
    {
      perf_test();
      return 0;
    }

  double max_delta = 0;
  for (int i = 0; i < 256; i++)
    {
      double s, c;
      sm_sincos (i / 256.0 * 2 * M_PI, &s, &c);

      const double is = int_sinf (i);
      const double ic = int_cosf (i);
//      printf ("%d %f %f %f %f\n", i, s, c, is, ic);
      max_delta = max (max_delta, fabs (s - is));
      max_delta = max (max_delta, fabs (c - ic));
    }
  printf ("int_sincos: max_delta = %g\n", max_delta);
  assert (max_delta < 1e-5);
}
