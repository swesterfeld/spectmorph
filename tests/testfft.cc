#include "smfft.hh"
#include "smrandom.hh"

#include <stdio.h>

using namespace SpectMorph;

int
main()
{
  SpectMorph::Random random;

  const int block_size = 4;

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

  return 0;
}
