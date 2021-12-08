// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "smmath.hh"
#include "smutils.hh"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  double mix_freq = sm_atof (argv[1]);
  double freq = sm_atof (argv[2]);
  double a = sm_lowpass1_factor (mix_freq, freq);

  printf ("# a=%f\n", a);

  double x = 0;
  for (double d = 0; d < mix_freq; d += 1)
    {
      x = a * 1.0 + (1 - a) * x;
      printf ("%f %f\n", d / mix_freq, x);
    }
}
