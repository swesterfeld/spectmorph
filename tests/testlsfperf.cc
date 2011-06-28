/*
 * Copyright (C) 2011 Stefan Westerfeld
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
#include "smlpc.hh"
#include "smmain.hh"
#include <sys/time.h>
#include <stdio.h>
#include <vector>

using namespace SpectMorph;
using std::vector;
using std::min;
using std::max;

float lpc_lsf_p[26] = {
  0.04010000079870224,
  0.10260000079870224,
  0.15979999303817749,
  0.36309999227523804,
  0.44119998812675476,
  0.52480000257492065,
  0.59769999980926514,
  0.84700000286102295,
  1.0470000505447388,
  1.1173000335693359,
  1.1878000497817993,
  1.3303999900817871,
  1.4661999940872192,
  1.5259000062942505,
  1.695099949836731,
  1.8607000112533569,
  1.9872000217437744,
  2.0701000690460205,
  2.2330999374389648,
  2.3413000106811523,
  2.4275000095367432,
  2.5488998889923096,
  2.7269999980926514,
  2.8210999965667725,
  2.9467000961303711,
  3.1415998935699463
};
float lpc_lsf_q[26] = {
  0,
  0.08959999680519104,
  0.14159999787807465,
  0.25479999184608459,
  0.4034000039100647,
  0.47389999032020569,
  0.54629999399185181,
  0.65079998970031738,
  0.946399986743927,
  1.0937000513076782,
  1.1654000282287598,
  1.235200047492981,
  1.3609999418258667,
  1.4850000143051147,
  1.625499963760376,
  1.7690999507904053,
  1.9355000257492065,
  2.0137999057769775,
  2.1136000156402588,
  2.2646000385284424,
  2.3945999145507812,
  2.4665999412536621,
  2.6508998870849609,
  2.7595999240875244,
  2.8538000583648682,
  2.9542999267578125
};

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

  vector<double> lpc;
  vector<float>  lsf_p (lpc_lsf_p, lpc_lsf_p + 26);
  vector<float>  lsf_q (lpc_lsf_q, lpc_lsf_q + 26);

  LPC::LSFEnvelope env;
  env.init (lsf_p, lsf_q);

  LPC::lsf2lpc (lsf_p, lsf_q, lpc);

  double clocks_per_sec = 2500.0 * 1000 * 1000;
  double best = 1e30;

  size_t runs = 100000;
  for (size_t reps = 0; reps < 10; reps++)
    {
      double start = gettime();
      double x = 0;
      for (size_t i = 0; i < runs; i++)
        x += env.eval (0.7);
      double end = gettime();
      best = min (end - start, best);
    }

  printf ("%f clocks/value\n", clocks_per_sec * best / runs);

  double max_diff = 0;
  for (float f = 0; f < M_PI; f += M_PI / 1000)
    {
      double mag_lpc = LPC::eval_lpc (lpc, f);
      double mag_lsf = env.eval (f);
      max_diff = max (max_diff, fabs (mag_lpc - mag_lsf) / max (mag_lpc, mag_lsf));
    }
  printf ("max relative diff [lpc/lsf] = %.17g\n", max_diff);
}
