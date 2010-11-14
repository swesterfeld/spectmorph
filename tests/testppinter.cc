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
using std::string;
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

void
sweep_test()
{
  PolyPhaseInter *ppi = PolyPhaseInter::the();
  const double SR = 48000;

  double freq = 5;
  double phase = 0;

  vector<float> sin_signal, cos_signal, freq_signal;

  while (freq < 24000)
    {
      sin_signal.push_back (sin (phase));
      cos_signal.push_back (cos (phase));
      freq_signal.push_back (freq);

      phase += 2 * M_PI * freq / SR;
      if (phase > 2 * M_PI)
        phase -= 2 * M_PI;
      freq += 0.1;
    }

  const double SPEED = 4.71;
  double pos = 100;
  while (pos < (sin_signal.size() - 100))
    {
      double si = ppi->get_sample (sin_signal, pos);
      double ci = ppi->get_sample (cos_signal, pos);
      double fi = ppi->get_sample (freq_signal, pos);
      printf ("%.17g %.17g\n", fi, sqrt (si * si + ci * ci));
      pos += SPEED;
    }
}

double
sinc (double x)
{
  if (fabs (x) < 1e-6)
    return 1;
  else
    return sin (M_PI * x) / (M_PI * x);
}

void
impulse_test()
{
  PolyPhaseInter *ppi = PolyPhaseInter::the();
  vector<float> one_signal (40);
  one_signal[20] = 1.0;

  const int FFT_SIZE = 256 * 1024;
  float *fft_in = FFT::new_array_float (FFT_SIZE);
  float *fft_out = FFT::new_array_float (FFT_SIZE);
  int k = 0;

  const double SR = 48000;
  const double LP_FREQ = 20000;
  const double STEP = 0.001;
  for (double p = -20; p < 20; p += STEP)
    {
      double c = sinc (double (p) / (SR / 2) * LP_FREQ);
      double w = bse_window_blackman (double (p) / 12) / (SR / 2) * LP_FREQ;
      double x = c * w;

      x = ppi->get_sample (one_signal, 20 + p);

      fft_in[k++] = x * STEP;
    }
  FFT::fftar_float (FFT_SIZE, fft_in, fft_out);
  fft_out[1] = 0;
  for (int i = 0; i < FFT_SIZE; i += 2)
    {
      double re = fft_out[i];
      double im = fft_out[i + 1];
      printf ("%d %.17g\n", i, sqrt (re * re + im * im));
    }
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  if (argc == 2 && string (argv[1]) == "sweep")
    {
      sweep_test();
    }
  else if (argc == 2 && string (argv[1]) == "impulse")
    {
      impulse_test();
    }
  else
    {
      sin_test (440, -85);
      sin_test (2000, -75);
    }
}
