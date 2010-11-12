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

#include "smmain.hh"
#include "smrandom.hh"
#include "smencoder.hh"
#include "smlivedecoder.hh"
#include "smfft.hh"

#include <bse/bsemathsignal.h>

#include <vector>
#include <assert.h>
#include <stdio.h>

using namespace SpectMorph;

using std::vector;
using std::max;
using std::min;

void
sin_test (double freq, double db_bound)
{
  PolyPhaseInter *ppi = PolyPhaseInter::the();

  double step[] = { 0.456, 0.567, 0.888, 0.901, 1.01, 1.2, 1.3, -1 };
  double error = 0;

  for (int s = 0; step[s] > 0; s++)
    {
      const double SR = 48000;
      vector<float> input (SR * 2);
      vector<float> expect (SR);
      vector<float> output (SR);

      for (size_t t = 0; t < input.size(); t++)
        input[t] = sin (t * 2 * M_PI * freq / SR);

      for (size_t t = 0; t < output.size(); t++)
        {
          output[t] = ppi->get_sample (input, step[s] * t);
          expect[t] = sin (t * step[s] * 2 * M_PI * freq / SR);
        }

      for (size_t p = 20; p < output.size() - 20; p++)
        {
          error = max (error, fabs (output[p] - expect[p]));
        }
    }
  printf ("%.17g %.17g %.17g\n", freq, error, bse_db_from_factor (error, -200));
  assert (error < bse_db_to_factor (db_bound));
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  sin_test (440, -85);
  sin_test (2000, -75);
}
