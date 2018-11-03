// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smwavsetbuilder.hh"
#include "sminstenccache.hh"

#include <assert.h>
#include <sys/time.h>

using namespace SpectMorph;
using std::vector;

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  assert (argc == 2);

  vector<double> times;

  for (int i = 0; i < 10; i++)
    {
      // pretend that the just program started, and cache doesn't have in-memory entries
      InstEncCache::the()->clear();

      double t = gettime();
      WavSet wav_set;
      Instrument inst;
      inst.load (argv[1]);
      WavSetBuilder builder (&inst);
      builder.run();
      builder.get_result (wav_set);
      times.push_back ((gettime() - t) * 1000);
    }

  // report times at end of test
  for (auto t_ms : times)
      printf ("time: %.2f ms\n", t_ms);
}
