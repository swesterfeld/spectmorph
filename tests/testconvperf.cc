// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <stdio.h>
#include <assert.h>

#include <vector>

#include "smmath.hh"
#include "smmain.hh"
#include "smutils.hh"

using namespace SpectMorph;
using std::vector;

double global_var;
int global_int;

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

  start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    global_var += sm_ifreq2freq (i);
  const double t_ifreq2freq = get_time() - start;

  start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    sm_freq2ifreq (7.342);
  const double t_freq2ifreq = get_time() - start;

  start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    global_var += sm_idb2factor (i);
  const double t_idb2factor = get_time() - start;

  start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    global_int += sm_factor2idb (i);
  const double t_factor2idb = get_time() - start;

  printf ("%9.4f ifreq2freq\n", clocks_per_sec * t_ifreq2freq / runs);
  printf ("%9.4f freq2ifreq\n", clocks_per_sec * t_freq2ifreq / runs);
  printf ("%9.4f idb2factor\n", clocks_per_sec * t_idb2factor / runs);
  printf ("%9.4f factor2idb\n", clocks_per_sec * t_factor2idb / runs);
}
