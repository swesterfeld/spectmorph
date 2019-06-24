// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smrandom.hh"
#include "smfft.hh"
#include "smmain.hh"
#include "smutils.hh"

#include <glib.h>

#include <stdio.h>
#include <string>

using namespace SpectMorph;
using std::string;

int
main (int argc, char **argv)
{
  SpectMorph::Random random;

  Main main (&argc, &argv);

  double clocks_per_sec = 2500.0 * 1000 * 1000;
  double start = get_time();

  const int runs = 500000;
  const int bs = 1024;
  for (int i = 0; i < runs; i++)
    {
      guint32 block[(bs + 3) / 4];

      guint8 *block_b = reinterpret_cast<guint8 *>(&block[0]);
      for (int b = 0; b < bs; b++)
        block_b[b] = random.random_uint32();
    }
  double end = get_time();
  printf ("%f clocks/value\n", clocks_per_sec * (end - start) / runs / bs);

  start = get_time();
  for (int i = 0; i < runs; i++)
    {
      guint32 block[(bs + 3) / 4];

      random.random_block (bs / 4, block);
#if 0
      for (int b = 0; b < bs / 4; b++)
        {
          printf ("0x%08x\n", block[b]);
        }
#endif
    }
  end = get_time();

  printf ("%f clocks/value\n", clocks_per_sec * (end - start) / runs / bs);
}
