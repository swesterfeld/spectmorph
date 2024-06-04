// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <stdio.h>
#include <assert.h>

#include <vector>

#include "smmath.hh"
#include "smmain.hh"
#include "smutils.hh"

using namespace SpectMorph;
using std::vector;

float global_f = 0;

double
perf (bool fl2, int N)
{
  vector<float> block (4096);
  float f = 0.01;
  const unsigned int runs = 10000000;
  double start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    {
      for (size_t i = 0; i < N; i++)
        {
          global_f += block[i];
          block[i] = f;
          f += 0.001;
        }
      if (fl2)
        fast_log2 (block.data(), N);
      else
        {
          for (size_t i = 0; i < N; i++)
            block[i] = log2f (block[i]);
        }
    }
  double end = get_time();
  return (end - start) / runs / N;
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  uint64_t K = 33452759; // prime
  vector<float> x (K);
  vector<float> expect (K);
  for (size_t k = 0; k < K; k++)
    {
      /* range 2^-20 .. 2^20 */
      double d = (double (k) / K) * 40 - 20;
      expect[k] = d;
      x[k] = exp2 (d);
    }
  fast_log2 (x.data(), x.size());
  double max_err = 0;
  for (size_t k = 0; k < K; k++)
    {
      max_err = std::max (max_err, std::abs (double (x[k]) - double (expect[k])));
    }
  sm_printf ("max_err = %f\n", max_err);

  for (int N = 1; N <= 1024; N *= 2)
    {
      sm_printf ("N=%d\n", N);
      sm_printf ("%9.4f fast_log2\n", 1e9 * perf (true, N));
      sm_printf ("%9.4f log2f\n", 1e9 * perf (false, N));
    }
}
