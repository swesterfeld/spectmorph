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

#include "smmath.hh"
#include "smmain.hh"

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

#include <algorithm>
#include <string>

using namespace SpectMorph;
using std::max;
using std::string;

static double acc[2] = { 0, 0 };

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void
perf_test()
{
  double clocks_per_sec = 2500.0 * 1000 * 1000;
  double start = gettime();
  static int CALLS = 1000 * 1000 * 1000;
  for (int i = 0; i < CALLS; i++)
    {
      double is, ic;
      int_sincos (i, &is, &ic);
      acc[0] += is;
      acc[1] += ic;
    }
  double end = gettime();
  printf ("int_sincos: %f clocks/invocation\n", clocks_per_sec * (end - start) / CALLS);
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  if (argc == 2 && string (argv[1]) == "perf")
    {
      perf_test();
      return 0;
    }

  double max_delta = 0;
  for (int i = 0; i < 256; i++)
    {
      double s, c, is, ic;
      sincos (i / 256.0 * 2 * M_PI, &s, &c);
      int_sincos (i, &is, &ic);
//      printf ("%d %f %f %f %f\n", i, s, c, is, ic);
      max_delta = max (max_delta, fabs (s - is));
      max_delta = max (max_delta, fabs (c - ic));
    }
  printf ("int_sincos: max_delta = %g\n", max_delta);
  assert (max_delta < 1e-5);
}
