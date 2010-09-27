/*
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smrandom.hh"
#include "smfft.hh"
#include "smmain.hh"

#include <glib.h>

#include <sys/time.h>
#include <stdio.h>
#include <string>

using namespace SpectMorph;
using std::string;

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
  SpectMorph::Random random;

  sm_init (&argc, &argv);

  double clocks_per_sec = 2500.0 * 1000 * 1000;
  double start = gettime();

  const int runs = 500000;
  const int bs = 1024;
  for (int i = 0; i < runs; i++)
    {
      guint32 block[(bs + 3) / 4];

      guint8 *block_b = reinterpret_cast<guint8 *>(&block[0]);
      for (int b = 0; b < bs; b++)
        block_b[b] = random.random_int32();
    }
  double end = gettime();
  printf ("%f clocks/value\n", clocks_per_sec * (end - start) / runs / bs);

  start = gettime();
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
  end = gettime();

  printf ("%f clocks/value\n", clocks_per_sec * (end - start) / runs / bs);
}
