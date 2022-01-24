// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smpandaresampler.hh"
#include "smalignedarray.hh"
#include "smutils.hh"

#include <vector>

using std::vector;
using PandaResampler::Resampler2;
using namespace SpectMorph;

double
perf (bool sse)
{
  AlignedArray<float, 16> in (512);
  AlignedArray<float, 16> out (in.size() * 2);

  Resampler2 ups (Resampler2::UP, 2, Resampler2::PREC_72DB, sse);
  Resampler2 downs (Resampler2::DOWN, 2, Resampler2::PREC_72DB, sse);

  double min_time = 1e20;
  const int RUNS = 20000, REPS = 13;
  for (int rep = 0; rep < REPS; rep++)
    {
      double t = get_time();
      for (int r = 0; r < RUNS; r++)
        {
          ups.process_block (&in[0], in.size(), &out[0]);
          downs.process_block (&out[0], out.size(), &in[0]);
        }
      min_time = std::min (get_time() - t, min_time);
    }

  const double ns_per_sec = 1e9;
  printf ("%s: %.2f ns/sample\n", sse ? "SSE" : "FPU", min_time * ns_per_sec / RUNS / in.size());

  return min_time;
}

int
main()
{
  double with_sse = perf (true);
  double with_fpu = perf (false);
  printf ("\nSSE/FPU speedup: %.2f\n", with_fpu/with_sse);
}
