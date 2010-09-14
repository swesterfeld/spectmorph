#include "smfft.hh"
#include "smrandom.hh"

#include <stdio.h>

using namespace SpectMorph;

int
main()
{
  SpectMorph::Random random;

  const int block_size = 8;

  float *in = FFT::new_array_float (block_size);
  float *out = FFT::new_array_float (block_size);
  float *out_gsl = FFT::new_array_float (block_size);
  float *back = FFT::new_array_float (block_size);
  float *back_gsl = FFT::new_array_float (block_size);

  for (int i = 0; i < block_size; i++)
    {
      in[i] = random.random_double_range (-1, 1);
      out[i] = 0;
      out_gsl[i] = 0;
    }
  FFT::use_gsl_fft (false);
  FFT::fftar_float (block_size, in, out);
  FFT::fftsr_float (block_size, out, back);

  FFT::use_gsl_fft (true);
  FFT::fftar_float (block_size, in, out_gsl);
  FFT::fftsr_float (block_size, out_gsl, back_gsl);

  printf ("data:\n");
  for (int i = 0; i < block_size; i++)
    printf ("%d %.8f\n", i, in[i]);

  printf ("fftar:\n");
  for (int i = 0; i < block_size; i++)
    printf ("%d %.8f %.8f %.8f\n", i, out[i], out_gsl[i], out_gsl[i] - out[i]);

  printf ("===\n");
  printf ("fftsr:\n");
  for (int i = 0; i < block_size; i++)
    printf ("%d %.17g %.17g %.17g\n", i, in[i], back[i], back_gsl[i]);

  FFT::use_gsl_fft (false);
  FFT::fftac_float (block_size / 2, in, out);
  FFT::fftsc_float (block_size / 2, out, back);

  FFT::use_gsl_fft (true);
  FFT::fftac_float (block_size / 2, in, out_gsl);
  FFT::fftsc_float (block_size / 2, out_gsl, back_gsl);

  printf ("data:\n");
  for (int i = 0; i < block_size; i += 2)
    printf ("%d %.8f %.8f\n", i / 2, in[i], in[i + 1]);
  printf ("fftac:\n");
  for (int i = 0; i < block_size; i += 2)
    printf ("fftw: %d %.8f %.8f\n", i / 2, out[i], out[i + 1]);
  for (int i = 0; i < block_size; i += 2)
    printf ("gslf: %d %.8f %.8f\n", i / 2, out_gsl[i], out_gsl[i + 1]);
  printf ("===\n");
  printf ("fftsc:\n");
  for (int i = 0; i < block_size; i++)
    printf ("%d %.17g %.17g %.17g\n", i, in[i], back[i], back_gsl[i]);
  return 0;
}
