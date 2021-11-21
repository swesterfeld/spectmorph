// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmath.hh"
#include <stdio.h>

using namespace SpectMorph;

int
main()
{
  for (int i = 0; i <= 100; i++)
    {
      double x = i / 100.;
      double value = sm_xparam (x, 3);
      double ivalue = sm_xparam_inv (value, 3);
      printf ("%f %f %f\n", x, value, ivalue);
    }
}
