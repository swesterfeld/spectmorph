// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smmath.hh"
#include <bse/bsemathsignal.hh>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using namespace SpectMorph;

using std::max;
using std::min;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  double emin = 0, emax = 0, econv = 0;
  for (size_t i = 0; i < 65536; i++)
    {
      econv = max (econv, (sm_idb2factor (i) - sm_idb2factor_slow (i)) / sm_idb2factor_slow (i));
    }
  const double conv_bound = 2e-7;
  printf ("conversion error%%: %.6f bound %.6f\n", econv * 100, conv_bound * 100);

  for (double factor = 0.1; factor < 10; factor += 0.00001)
    {
      int16_t idb = sm_factor2idb (factor);
      double  xfactor = sm_idb2factor (idb);
      double error =  (factor - xfactor) / factor;

      emin = min (error, emin);
      emax = max (error, emax);

      // printf ("%f %f %d %f %f\n", factor, bse_db_from_factor (factor, -500), idb, xfactor, error);
    }
  const double bound = 0.0009;
  printf ("representation error%%: [%.6f, %.6f] bound %.6f\n", emin * 100, emax * 100, bound * 100);

  assert (econv < conv_bound);
  assert (-emin < bound);
  assert (emax  < bound);
}
