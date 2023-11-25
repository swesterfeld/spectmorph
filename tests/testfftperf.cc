// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smrandom.hh"
#include "smfft.hh"
#include "smmain.hh"
#include "smutils.hh"
#include <stdio.h>
#include <string>

using namespace SpectMorph;
using std::string;

unsigned int block_size;
float *in, *out;

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
  double ns_per_sec = 1e9;
  unsigned int runs = 100000;

  // warmup run:
  func();

  // timed runs:
  double start = get_time();
  for (unsigned int i = 0; i < runs; i++)
    func();
  double end = get_time();
  printf ("%s: %f ns/sample\n", name.c_str(), ns_per_sec * (end - start) / (is_complex ? (block_size / 2) : block_size) / runs);

  return (end - start);
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  for (block_size = 256; block_size < 8192; block_size *= 2)
    {
      in = FFT::new_array_float (block_size);
      out = FFT::new_array_float (block_size);

      SpectMorph::Random random;
      for (unsigned int i = 0; i < block_size; i++)
        in[i] = random.random_double_range (-1, 1);

      printf ("========================= BLOCK_SIZE = %d =========================\n", block_size);

      measure ("fftar", time_fftar, false);
      measure ("fftsr", time_fftsr, false);

      measure ("fftac", time_fftac, true);
      measure ("fftsc", time_fftsc, true);

      FFT::free_array_float (in);
      FFT::free_array_float (out);
    }

}
