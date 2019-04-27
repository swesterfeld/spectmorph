// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smoutfile.hh"
#include "sminfile.hh"
#include "smgenericin.hh"
#include "smmemout.hh"
#include "smutils.hh"

#include <glib.h>

#include <stdlib.h>
#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::min;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  double total = 1e7;
  for (size_t rep = 0; rep < 10; rep++)
    {
      vector<unsigned char> data;
      data.reserve (6 * 10000 * 1000);
      MemOut  mem_out (&data);

      OutFile outfile (&mem_out, "SpectMorph::TestBlob", 42);

      vector<float> fblock;
      for (size_t i = 0; i < 1000; i++)
        fblock.push_back (g_random_double_range (-1, 1));

      outfile.write_float_block ("foo", fblock); // warmup

      double start = get_time();
      for (size_t i = 0; i < 10000; i++)
        {
          outfile.write_float_block ("foo", fblock);
        }
      double end = get_time();
      total = min (total, end - start);
    }

  printf ("data rate = %.2f Mb/sec\n", (4 * 10000 * 1000) / total / 1024 / 1024);
}
