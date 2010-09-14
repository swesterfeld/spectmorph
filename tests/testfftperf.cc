#include "smrandom.hh"
#include "smfft.hh"
#include <sys/time.h>
#include <stdio.h>
#include <string>

using namespace SpectMorph;
using std::string;

unsigned int block_size;
float *in, *out;

double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void
time_fftar()
{
  FFT::fftar_float (block_size, in, out);
}

void
time_fftsr()
{
  FFT::fftsr_float (block_size, in, out);
}

double
measure (const string& name, void (*func)())
{
  double clocks_per_sec = 2500.0 * 1000 * 1000;
  unsigned int runs = 10000;

  // warmup run:
  func();

  // timed runs:
  double start = gettime();
  for (int i = 0; i < runs; i++)
    func();
  double end = gettime();
  printf ("%s: %f clocks/sample\n", name.c_str(), clocks_per_sec * (end - start) / block_size / runs);

  return (end - start);
}

void
compare (const string& name, void (*func)())
{
  FFT::use_gsl_fft (false);
  double fftw = measure (name + "(fftw)", func);

  FFT::use_gsl_fft (true);
  double gsl = measure (name + "(gsl)", func);

  printf (" => speedup: %.3f\n", gsl / fftw);
  printf ("\n");
}

int
main()
{
  for (block_size = 256; block_size < 8192; block_size *= 2)
    {
      in = FFT::new_array_float (block_size);
      out = FFT::new_array_float (block_size);

      SpectMorph::Random random;
      for (int i = 0; i < block_size; i++)
        in[i] = random.random_double_range (-1, 1);

      printf ("========================= BLOCK_SIZE = %d =========================\n", block_size);

      compare ("fftar", time_fftar);
      compare ("fftsr", time_fftsr);

      FFT::free_array_float (in);
      FFT::free_array_float (out);
    }

}
