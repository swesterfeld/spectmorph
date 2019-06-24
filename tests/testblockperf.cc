// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smrandom.hh"
#include "smfft.hh"
#include "smblockutils.hh"
#include "smalignedarray.hh"

#include <assert.h>

using namespace SpectMorph;
using std::max;
using std::min;

static void
block_perf (bool add, bool aligned)
{
  const size_t block_size = 4096;

  AlignedArray<float, 16> a (block_size + 1);
  AlignedArray<float, 16> b (block_size + 1);

  // FIXME: could test with and without
  // sm_enable_sse (sse);

  Random random;
  random.set_seed (42);
  for (size_t i = 0; i < block_size; i++)
    {
      a[i] = random.random_double_range (-1.0, 1.0);
      if (add)
        b[i] = random.random_double_range (-1.0, 1.0);
      else
        b[i] = random.random_double_range (1.0, 1.0001);
    }
  double min_time = 1e20;
  const int RUNS = 20000, REPS = 13;
  for (int reps = 0; reps < REPS; reps++)
    {
      double start = get_time();
      for (int r = 0; r < RUNS; r++)
        {
          if (add)
            Block::add (block_size, &a[aligned ? 0 : 1], &b[0]);
          else
            Block::mul (block_size, &a[aligned ? 0 : 1], &b[0]);
        }
      double end = get_time();
      min_time = min (min_time, end - start);
    }

  const double ns_per_sec = 1e9;
  const double time_norm = ns_per_sec / RUNS / block_size;

  printf ("%s %s %f ns/sample\n", add ? "add" : "mul", aligned ? "  aligned" : "unaligned", min_time * time_norm);
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  block_perf (true,  false);
  block_perf (false, false);
  printf ("------------------------\n");
  block_perf (true,  true);
  block_perf (false, true);
}
