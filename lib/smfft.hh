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

enum PlanMode { PLAN_PATIENT, PLAN_ESTIMATE };

void   fftar_float (size_t N, float *in, float *out, PlanMode plan_mode = PLAN_PATIENT);
void   fftsr_float (size_t N, float *in, float *out, PlanMode plan_mode = PLAN_PATIENT);
void   fftsr_destructive_float (size_t N, float *in, float *out, PlanMode plan_mode = PLAN_PATIENT);
void   fftac_float (size_t N, float *in, float *out, PlanMode plan_mode = PLAN_PATIENT);
void   fftsc_float (size_t N, float *in, float *out, PlanMode plan_mode = PLAN_PATIENT);

void   use_gsl_fft (bool enabled);
void   debug_randomize_new_arrays (bool enabled);

void   init();
void   cleanup();

}

}

#endif
