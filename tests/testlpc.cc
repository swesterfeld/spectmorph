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

#include "smlpc.hh"
#include "smmain.hh"
#include "smwavloader.hh"
#include <assert.h>
#include <math.h>
#include <stdio.h>

using namespace SpectMorph;

using std::vector;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  assert (argc == 2);

  WavLoader *wav_loader = WavLoader::load (argv[1]);
  assert (wav_loader);

  const vector<float>& samples = wav_loader->samples();

  vector<double> lpc (50);
  vector<float> lsf_p, lsf_q;

  LPC::compute_lpc (lpc, &samples[0], &samples[samples.size()]);
  LPC::lpc2lsf (lpc, lsf_p, lsf_q);
  assert (lsf_p.size() == lsf_q.size());

  LPC::LSFEnvelope env (lsf_p, lsf_q);
  for (double freq = 0; freq < M_PI; freq += 0.001)
    {
      double lpc_value = LPC::eval_lpc (lpc, freq);
      double value = env.eval (freq);
      printf ("%f %.17g %.17g\n", freq / (2 * M_PI) * 44100, value, lpc_value);
    }
  delete wav_loader;
}
