// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <sys/time.h>
#include <stdio.h>
#include <assert.h>

#include "smmath.hh"
#include "smmain.hh"

using namespace SpectMorph;
using std::vector;

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

double global_var;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  const unsigned int runs = 10000000;
  const double clocks_per_sec = 2500.0 * 1000 * 1000;
  double start;

  // warmup
  sm_ifreq2freq (12345);
  sm_freq2ifreq (7.342);
  sm_idb2factor (12345);
  sm_factor2idb (7.342);

  start = gettime();
  for (unsigned int i = 0; i < runs; i++)
    global_var += sm_ifreq2freq (i);
  const double t_ifreq2freq = gettime() - start;

  start = gettime();
  for (unsigned int i = 0; i < runs; i++)
    sm_freq2ifreq (7.342);
  const double t_freq2ifreq = gettime() - start;

  start = gettime();
  for (unsigned int i = 0; i < runs; i++)
    global_var += sm_idb2factor (i);
  const double t_idb2factor = gettime() - start;

  start = gettime();
  for (unsigned int i = 0; i < runs; i++)
    sm_factor2idb (7.342);
  const double t_factor2idb = gettime() - start;

  printf ("%9.4f ifreq2freq\n", clocks_per_sec * t_ifreq2freq / runs);
  printf ("%9.4f freq2ifreq\n", clocks_per_sec * t_freq2ifreq / runs);
  printf ("%9.4f idb2factor\n", clocks_per_sec * t_idb2factor / runs);
  printf ("%9.4f factor2idb\n", clocks_per_sec * t_factor2idb / runs);
}
