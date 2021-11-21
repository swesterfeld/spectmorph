// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smrandom.hh"
#include <stdio.h>

int
main()
{
  SpectMorph::Random random;
  printf ("random (10000, 20000) = %f\n", random.random_double_range (10000, 20000));
  printf ("deterministic (-1 .. 1)\n");

  random.set_seed (42);
  for (int i = 0; i < 5; i++)
    printf ("%f\n", random.random_double_range (-1, 1));
}
