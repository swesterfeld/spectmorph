// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmath.hh"
#include <algorithm>
#include <stdio.h>
#include <assert.h>
#include "smutils.hh"
#include "smalignedarray.hh"
#include <string>

using namespace SpectMorph;
using std::string;

void
perftest()
{
  const size_t TEST_SIZE = 3001;
  int REPS = 20000;

  double sin_vec[TEST_SIZE], cos_vec[TEST_SIZE];
  AlignedArray<float, 16> sin_vecf (TEST_SIZE), cos_vecf (TEST_SIZE);
  double start_time, end_time;

  volatile double yy = 0;

  VectorSinParams params;

  params.freq = 440;
  params.mix_freq = 44100;
  params.phase = 0.12345;
  params.mag = 0.73;
  params.mode = VectorSinParams::REPLACE;

  start_time = get_time();
  for (int reps = 0; reps < REPS; reps++)
    {
      for (size_t i = 0; i < TEST_SIZE; i++)
        {
          const double phase = params.freq / params.mix_freq * 2.0 * M_PI * i + params.phase;
          double s, c;
          sm_sincos (phase, &s, &c);
          sin_vec[i] = s * params.mag;
          cos_vec[i] = c * params.mag;
        }
      yy += sin_vec[reps & 1023] + cos_vec[reps & 2047]; // do not optimize loop out
    }
  end_time = get_time();

  // assume 2Ghz processor and compute cycles per value
  printf ("libm sincos: %f pseudocycles per value\n", (end_time - start_time) * 2.0 * 1000 * 1000 * 1000 / (TEST_SIZE * REPS));

  REPS *= 20; // optimized versions are a lot faster
  start_time = get_time();
  for (int reps = 0; reps < REPS; reps++)
    {
      fast_vector_sincos (params, sin_vec, sin_vec + TEST_SIZE, cos_vec);

      yy += sin_vec[reps & 1023] + cos_vec[reps & 2047]; // do not optimize loop out
    }
  end_time = get_time();

  // assume 2Ghz processor and compute cycles per value
  printf ("fast double sincos: %f pseudocycles per value\n", (end_time - start_time) * 2.0 * 1000 * 1000 * 1000 / (TEST_SIZE * REPS));

  start_time = get_time();
  for (int reps = 0; reps < REPS; reps++)
    {
      fast_vector_sincosf (params, &sin_vecf[0], &sin_vecf[TEST_SIZE], &cos_vecf[0]);

      yy += sin_vecf[reps & 1023] + cos_vecf[reps & 2047]; // do not optimize loop out
    }
  end_time = get_time();

  // assume 2Ghz processor and compute cycles per value
  printf ("fast float sincos: %f pseudocycles per value\n", (end_time - start_time) * 2.0 * 1000 * 1000 * 1000 / (TEST_SIZE * REPS));

  REPS *= 2; // sin version: even faster

  start_time = get_time();
  for (int reps = 0; reps < REPS; reps++)
    {
      fast_vector_sinf (params, &sin_vecf[0], &sin_vecf[TEST_SIZE]);

      yy += sin_vecf[reps & 1023]; // do not optimize loop out
    }
  end_time = get_time();

  // assume 2Ghz processor and compute cycles per value
  printf ("fast float sin: %f pseudocycles per value\n", (end_time - start_time) * 2.0 * 1000 * 1000 * 1000 / (TEST_SIZE * REPS));
}

int
main (int argc, char **argv)
{
  // do perf test only when the user wants it

  if (argc == 2 && string (argv[1]) == "perf")
    perftest();

  // always do accuracy test

  double freqs[] = { 40, 440, 880, 1024, 3000, 9000, 12345, -1 };
  double rates[] = { 44100, 48000, 96000, -1 };
  double phases[] = { -0.9, 0.0, 0.3, M_PI * 0.5, M_PI * 0.95, -100 };
  double mags[] = { 0.01, 0.1, 0.54321, 1.0, 2.24, -1 };

  const size_t TEST_SIZE = 3001;  // not dividable by 4 to expose SSE corner case code

  double small_random[TEST_SIZE];
  for (size_t i = 0; i < TEST_SIZE; i++)
    small_random[i] = g_random_double_range (-0.01, 0.01);

  double max_err = 0.0;
  double max_err_f = 0.0;

  for (int f = 0; freqs[f] > 0; f++)
    {
      for (int r = 0; rates[r] > 0; r++)
        {
          for (int p = 0; phases[p] > -99; p++)
            {
              for (int m = 0; mags[m] > 0; m++)
                {
                  VectorSinParams params;

                  params.freq = freqs[f];
                  params.mix_freq = rates[r];
                  params.phase = phases[p];
                  params.mag = mags[m];
                  params.mode = VectorSinParams::REPLACE;

                  // compute correct result
                  double libm_sin_vec[TEST_SIZE], libm_cos_vec[TEST_SIZE];

                  for (size_t i = 0; i < TEST_SIZE; i++)
                    {
                      const double phase = params.freq / params.mix_freq * 2.0 * M_PI * i + params.phase;
                      double s, c;
                      sm_sincos (phase, &s, &c);
                      libm_sin_vec[i] = s * params.mag;
                      libm_cos_vec[i] = c * params.mag;
                    }

                  // test fast double approximations
                  double sin_vec[TEST_SIZE], cos_vec[TEST_SIZE];

                  // random data should be overwritten
                  std::copy (small_random, small_random + TEST_SIZE, sin_vec);
                  fast_vector_sin (params, sin_vec, sin_vec + TEST_SIZE);

                  for (size_t i = 0; i < TEST_SIZE; i++)
                    max_err = std::max (max_err, fabs (sin_vec[i] - libm_sin_vec[i]));

                  assert (max_err < 1e-5);

                  // random data should be overwritten
                  std::copy (small_random, small_random + TEST_SIZE, sin_vec);
                  std::copy (small_random, small_random + TEST_SIZE, cos_vec);
                  fast_vector_sincos (params, sin_vec, sin_vec + TEST_SIZE, cos_vec);

                  for (size_t i = 0; i < TEST_SIZE; i++)
                    {
                      max_err = std::max (max_err, fabs (sin_vec[i] - libm_sin_vec[i]));
                      max_err = std::max (max_err, fabs (cos_vec[i] - libm_cos_vec[i]));
                    }
                  assert (max_err < 1e-5);

                  // test fast float approximations
                  AlignedArray<float, 16> sin_vecf (TEST_SIZE);
                  AlignedArray<float, 16> cos_vecf (TEST_SIZE);

                  // random data should be overwritten
                  std::copy (small_random, small_random + TEST_SIZE, &sin_vecf[0]);
                  fast_vector_sinf (params, &sin_vecf[0], &sin_vecf[TEST_SIZE]);

                  for (size_t i = 0; i < TEST_SIZE; i++)
                    max_err_f = std::max (max_err_f, fabs (sin_vecf[i] - libm_sin_vec[i]));

                  assert (max_err_f < 1e-5);

                  // random data should be overwritten
                  std::copy (small_random, small_random + TEST_SIZE, &sin_vecf[0]);
                  std::copy (small_random, small_random + TEST_SIZE, &cos_vecf[0]);
                  fast_vector_sincosf (params, &sin_vecf[0], &sin_vecf[TEST_SIZE], &cos_vecf[0]);

                  for (size_t i = 0; i < TEST_SIZE; i++)
                    {
                      max_err_f = std::max (max_err_f, fabs (sin_vecf[i] - libm_sin_vec[i]));
                      max_err_f = std::max (max_err_f, fabs (cos_vecf[i] - libm_cos_vec[i]));
                    }

                  assert (max_err_f < 1e-5);

                  // test mode = ADD for doubles
                  params.mode = VectorSinParams::ADD;

                  // random data should be preserved
                  std::copy (small_random, small_random + TEST_SIZE, sin_vec);
                  fast_vector_sin (params, sin_vec, sin_vec + TEST_SIZE);

                  for (size_t i = 0; i < TEST_SIZE; i++)
                    max_err = std::max (max_err, fabs (sin_vec[i] - small_random[i] - libm_sin_vec[i]));

                  assert (max_err < 1e-5);

                  // random data should be preserved
                  std::copy (small_random, small_random + TEST_SIZE, sin_vec);
                  std::copy (small_random, small_random + TEST_SIZE, cos_vec);
                  fast_vector_sincos (params, sin_vec, sin_vec + TEST_SIZE, cos_vec);

                  for (size_t i = 0; i < TEST_SIZE; i++)
                    {
                      max_err = std::max (max_err, fabs (sin_vec[i] - small_random[i] - libm_sin_vec[i]));
                      max_err = std::max (max_err, fabs (cos_vec[i] - small_random[i] - libm_cos_vec[i]));
                    }

                  assert (max_err < 1e-5);

                  // test mode = ADD for floats

                  // random data should be preserved
                  std::copy (small_random, small_random + TEST_SIZE, &sin_vecf[0]);
                  fast_vector_sinf (params, &sin_vecf[0], &sin_vecf[TEST_SIZE]);

                  for (size_t i = 0; i < TEST_SIZE; i++)
                    max_err_f = std::max (max_err_f, fabs (sin_vecf[i] - small_random[i] - libm_sin_vec[i]));

                  assert (max_err_f < 1e-5);

                  // random data should be preserved
                  std::copy (small_random, small_random + TEST_SIZE, &sin_vecf[0]);
                  std::copy (small_random, small_random + TEST_SIZE, &cos_vecf[0]);
                  fast_vector_sincosf (params, &sin_vecf[0], &sin_vecf[TEST_SIZE], &cos_vecf[0]);

                  for (size_t i = 0; i < TEST_SIZE; i++)
                    {
                      max_err_f = std::max (max_err_f, fabs (sin_vecf[i] - small_random[i] - libm_sin_vec[i]));
                      max_err_f = std::max (max_err_f, fabs (cos_vecf[i] - small_random[i] - libm_cos_vec[i]));
                    }

                  assert (max_err_f < 1e-5);
                }
            }
        }
    }
  printf ("testfastsin: maximum double error: %.17g\n", max_err);
  printf ("testfastsin: maximum float error: %.17g\n", max_err_f);
  assert (max_err < 5e-12);
  assert (max_err_f < 7e-7);
}
