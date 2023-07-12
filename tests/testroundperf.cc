// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <stdio.h>

#include "smmath.hh"
#include "smmain.hh"
#include "smutils.hh"

using namespace SpectMorph;
using std::vector;

int global_int;

float f[4] = { 0.2f, 1.2f, 0.8f, 2.3f };
double d[4] = { 0.2, 1.2, 0.8, 2.3 };

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  const unsigned int runs = 1'000'000'000;
  double start;

  start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    {
      global_int += sm_round_positive (f[i & 3]);
      f[i & 3] += 1e-7f;
    }
  const double t_float = get_time() - start;

  start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    {
      global_int += sm_round_positive (d[i & 3]);
      d[i & 3] += 1e-7;
    }
  const double t_double = get_time() - start;

  start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    {
      global_int += int (d[i & 3]);
      d[i & 3] += 1e-7;
    }
  const double t_double_trunc = get_time() - start;

  const double ns_per_sec = 1e9;
  printf ("%.4f ns/round float\n", ns_per_sec * t_float / runs);
  printf ("%.4f ns/round double\n", ns_per_sec * t_double / runs);
  printf ("%.4f ns/trunc double\n", ns_per_sec * t_double_trunc / runs);
}
