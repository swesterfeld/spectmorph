// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_FFT_HH
#define SPECTMORPH_FFT_HH

#include <sys/types.h>

namespace SpectMorph
{

namespace FFT
{

float *new_array_float (size_t N);
void   free_array_float (float *f);

void   fftar_float (size_t N, float *in, float *out);
void   fftsr_float (size_t N, float *in, float *out);
void   fftsr_destructive_float (size_t N, float *in, float *out);
void   fftac_float (size_t N, float *in, float *out);
void   fftsc_float (size_t N, float *in, float *out);

void   use_gsl_fft (bool enabled);

void   load_wisdom();

}

}

#endif
