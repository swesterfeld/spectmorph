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
      econv = max (econv, (sm_ifreq2freq (i) - sm_ifreq2freq_slow (i)) / sm_ifreq2freq_slow (i));
    }
  const double conv_bound = 2e-7;
  printf ("conversion error%%: %.6f bound %.6f\n", econv * 100, conv_bound * 100);

  double base_freq = 15;
  const double CENT_FACTOR = pow (2, 1. / 1200);

  for (float freq = 0.125; freq < 20000. / base_freq; freq *= 1.01)
    {
      uint16_t ifreq   = sm_freq2ifreq (freq);
      double   xfreq   = sm_ifreq2freq (ifreq);
      double   cent_error   = (log (xfreq) - log (freq)) / log (CENT_FACTOR);
      //printf ("%.17g %.17g %.17g %d\n", freq * base_freq, xfreq * base_freq, cent_error, ifreq);
      emin = min (cent_error, emin);
      emax = max (cent_error, emax);
    }
  printf ("representation error [cent]: [%.3f, %.3f]\n", emin, emax);
}
