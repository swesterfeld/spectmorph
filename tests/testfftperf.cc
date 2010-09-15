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
#include <sys/time.h>
#include <stdio.h>
#include <string>
#include <bse/bseblockutils.hh>

using namespace SpectMorph;
using std::string;

unsigned int block_size;
float *in, *out;

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static void
time_fftar()
{
  FFT::fftar_float (block_size, in, out);
}

static void
time_fftsr()
{
  FFT::fftsr_float (block_size, in, out);
}

static void
time_fftac()
{
  FFT::fftac_float (block_size / 2, in, out);
}

static void
time_fftsc()
{
  FFT::fftsc_float (block_size / 2, in, out);
}

static double
measure (const string& name, void (*func)(), bool is_complex)
{
  double clocks_per_sec = 2500.0 * 1000 * 1000;
  unsigned int runs = 10000;

  // warmup run:
  func();

  // timed runs:
  double start = gettime();
  for (unsigned int i = 0; i < runs; i++)
    func();
  double end = gettime();
  printf ("%s: %f clocks/sample\n", name.c_str(), clocks_per_sec * (end - start) / (is_complex ? (block_size / 2) : block_size) / runs);

  return (end - start);
}

static void
compare (const string& name, void (*func)(), bool is_complex)
{
  FFT::use_gsl_fft (false);
  double fftw = measure (name + "(fftw)", func, is_complex);

  FFT::use_gsl_fft (true);
  double gsl = measure (name + "(gsl)", func, is_complex);

  printf (" => speedup: %.3f\n", gsl / fftw);
  printf ("\n");
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  printf ("Block Utils Implementation: %s\n", bse_block_impl_name());

  for (block_size = 256; block_size < 8192; block_size *= 2)
    {
      in = FFT::new_array_float (block_size);
      out = FFT::new_array_float (block_size);

      SpectMorph::Random random;
      for (unsigned int i = 0; i < block_size; i++)
        in[i] = random.random_double_range (-1, 1);

      printf ("========================= BLOCK_SIZE = %d =========================\n", block_size);

      compare ("fftar", time_fftar, false);
      compare ("fftsr", time_fftsr, false);

      compare ("fftac", time_fftac, true);
      compare ("fftsc", time_fftsc, true);

      FFT::free_array_float (in);
      FFT::free_array_float (out);
    }

}
