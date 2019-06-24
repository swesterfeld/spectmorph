// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smfft.hh"
#include "smrandom.hh"
#include "smmain.hh"
#include "smmath.hh"

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <algorithm>

using namespace SpectMorph;

using std::max;
using std::min;

static double
compare (int block_size, float *a, float *b)
{
  float delta = 0;
  float a_max = 0;
  float b_max = 0;

  for (int i = 0; i < block_size; i++)
    {
      a_max = max (a_max, fabs (a[i]));
      b_max = max (b_max, fabs (b[i]));
    }
  assert (a_max > 0);
  assert (b_max > 0);
  float max_abs_value = max (a_max, b_max);

  // ensure that a_max and b_max do not differ too much
  delta = max (delta, fabs (a_max - b_max) / min (a_max, b_max));

  // compare values (scaled by max value)
  for (int i = 0; i < block_size; i++)
    {
      delta = max (delta, fabs (a[i] - b[i]) / max_abs_value);
    }
  return delta;
}

int
main (int argc, char **argv)
{
  SpectMorph::Random random;

  Main main (&argc, &argv);

  for (int block_size = 8; block_size < 2048; block_size *= 2)
    {
      printf (" *** block_size  = %d ***\n\n", block_size);
      float *in = FFT::new_array_float (block_size);
      float *in_x = FFT::new_array_float (block_size * 2);
      float *out = FFT::new_array_float (block_size);
      float *out_x = FFT::new_array_float (block_size * 2);
      float *out_gsl = FFT::new_array_float (block_size);
      float *back = FFT::new_array_float (block_size);
      float *back_gsl = FFT::new_array_float (block_size);

      double max_delta;

      for (int i = 0; i < block_size; i++)
        {
          in[i] = random.random_double_range (-1, 1);
          out[i] = 0;
          out_gsl[i] = 0;
        }
      FFT::use_gsl_fft (false);
      FFT::fftar_float (block_size, in, out);
      FFT::fftsr_float (block_size, out, back);

      for (int k = 0; k < block_size; k++)
        {
          in_x[k * 2] = in[k];
          in_x[k * 2 + 1] = 0;
        }
      FFT::fftac_float (block_size, in_x, out_x);
      out_x[1] = out_x[block_size];

      FFT::use_gsl_fft (true);
      FFT::fftar_float (block_size, in, out_gsl);
      FFT::fftsr_float (block_size, out_gsl, back_gsl);

      /* check real results: */
      max_delta = compare (block_size, out, out_gsl);
      printf ("     FFTAR delta = %g\n", max_delta);
      assert (max_delta < 1e-6);   /* approximately 20 bit precision is good enough for us */

      max_delta = compare (block_size, back, back_gsl);
      printf ("     FFTSR delta = %g\n", max_delta);
      assert (max_delta < 1e-6);   /* approximately 20 bit precision is good enough for us */

      max_delta = compare (block_size, out_x, out_gsl);
      printf ("     FFTAR vs. FFTW real delta = %g\n", max_delta);
      assert (max_delta < 1e-6);   /* approximately 20 bit precision is good enough for us */

      FFT::use_gsl_fft (false);
      FFT::fftac_float (block_size / 2, in, out);
      FFT::fftsc_float (block_size / 2, out, back);

      FFT::use_gsl_fft (true);
      FFT::fftac_float (block_size / 2, in, out_gsl);
      FFT::fftsc_float (block_size / 2, out_gsl, back_gsl);

      /* check complex results: */
      max_delta = compare (block_size, out, out_gsl);
      printf ("     FFTAC delta = %g\n", max_delta);
      assert (max_delta < 1e-6);   /* approximately 20 bit precision is good enough for us */

      max_delta = compare (block_size, back, back_gsl);
      printf ("     FFTSC delta = %g\n", max_delta);
      assert (max_delta < 1e-6);   /* approximately 20 bit precision is good enough for us */

      printf ("\n");
    }

  return 0;
}
