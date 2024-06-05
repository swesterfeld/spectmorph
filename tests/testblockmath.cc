// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <stdio.h>
#include <assert.h>

#include <vector>

#include "smmath.hh"
#include "smmain.hh"
#include "smutils.hh"

using namespace SpectMorph;
using std::vector;

float global_f = 0;

double
perf (bool fl2, unsigned int N)
{
  vector<float> block (N);
  float f = 0.01;
  const unsigned int runs = 200'000'000 / std::max<uint> (N, 16);
  double start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    {
      for (size_t i = 0; i < N; i++)
        {
          global_f += block[i];
          block[i] = f;
          f += 0.001f;
        }
      if (fl2)
        fast_log2 (block.data(), N);
      else
        {
          for (size_t i = 0; i < N; i++)
            block[i] = log2f (block[i]);
        }
    }
  double end = get_time();
  return (end - start) / runs / N;
}

int global_i;

double
freq_perf (bool fast, unsigned int N)
{
  vector<double>   freqs (N);
  vector<uint16_t> ifreqs (N);
  for (size_t i = 0; i < freqs.size(); i++)
    freqs[i] = sm_ifreq2freq (i % 12345 + 3456);

  const unsigned int runs = 200'000'000 / std::max<uint> (N, 16);
  double start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    {
      for (size_t i = 0; i < N; i++)
        {
          global_i += ifreqs[i];
        }
      if (fast)
        sm_freq2ifreqs (freqs.data(), freqs.size(), ifreqs.data());
      else
        {
          for (size_t i = 0; i < N; i++)
            ifreqs[i] = sm_freq2ifreq (freqs[i]);
        }
    }
  double end = get_time();
  return (end - start) / runs / N;
}

double
mag_perf (bool fast, unsigned int N)
{
  vector<double>   mags (N);
  vector<uint16_t> imags (N);
  for (size_t i = 0; i < mags.size(); i++)
    mags[i] = sm_idb2factor (i % 12345 + 3456);

  const unsigned int runs = 200'000'000 / std::max<uint> (N, 16);
  double start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    {
      for (size_t i = 0; i < N; i++)
        {
          global_i += imags[i];
        }
      if (fast)
        sm_factor2idbs (mags.data(), mags.size(), imags.data());
      else
        {
          for (size_t i = 0; i < N; i++)
            imags[i] = sm_factor2idb (mags[i]);
        }
    }
  double end = get_time();
  return (end - start) / runs / N;
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  if (argc == 1)
    {
      uint64_t K = 33452759; // prime
      vector<float> x (K);
      vector<float> expect (K);
      for (size_t k = 0; k < K; k++)
        {
          /* range 2^-20 .. 2^20 */
          double d = (double (k) / K) * 40 - 20;
          expect[k] = d;
          x[k] = exp2 (d);
        }
      fast_log2 (x.data(), x.size());
      double max_err = 0;
      for (size_t k = 0; k < K; k++)
        {
          max_err = std::max (max_err, std::abs (double (x[k]) - double (expect[k])));
        }
      sm_printf ("max_err = %g\n", max_err);
      assert (max_err < 3.82e-6);

      vector<double> freqs (65536);
      vector<uint16_t> ifreqs (65536);
      for (size_t i = 0; i < freqs.size(); i++)
        freqs[i] = sm_ifreq2freq (i);
      sm_freq2ifreqs (freqs.data(), freqs.size(), ifreqs.data());
      for (size_t i = 0; i < freqs.size(); i++)
        {
          assert (i == ifreqs[i]);
          // printf ("F %zd -> %f -> %d\n", i, freqs[i], ifreqs[i]);
        }

      vector<double> mags (65536);
      vector<uint16_t> imags (65536);
      for (size_t i = 0; i < mags.size(); i++)
        mags[i] = sm_idb2factor (i);
      sm_factor2idbs (mags.data(), mags.size(), imags.data());
      for (size_t i = 0; i < mags.size(); i++)
        {
          assert (imags[i] == sm_factor2idb (mags[i]));
          // printf ("M %zd -> %f -> %d\n", i, mags[i], imags[i]);
        }
    }
  if (argc == 2 && !strcmp (argv[1], "perf"))
    {
      for (int N = 1; N <= 1024; N *= 2)
        {
          sm_printf ("N=%d\n", N);
          sm_printf ("%9.4f fast_log2\n", 1e9 * perf (true, N));
          sm_printf ("%9.4f log2f\n", 1e9 * perf (false, N));
          sm_printf ("%9.4f freq2ifreqs (block)\n", 1e9 * freq_perf (true, N));
          sm_printf ("%9.4f freq2ifreq\n", 1e9 * freq_perf (false, N));
          sm_printf ("%9.4f factor2idbs (block)\n", 1e9 * mag_perf (true, N));
          sm_printf ("%9.4f factor2idb\n", 1e9 * mag_perf (false, N));
        }
    }
}
