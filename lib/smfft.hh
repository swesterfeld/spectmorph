// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_FFT_HH
#define SPECTMORPH_FFT_HH

#include <sys/types.h>
#include <fftw3.h>
#include "smutils.hh"

namespace SpectMorph
{

namespace FFT
{

typedef fftwf_plan Plan;

float *new_array_float (size_t N);
void   free_array_float (float *f);

enum PlanMode { PLAN_PATIENT, PLAN_ESTIMATE };

void   fftar_float (size_t N, float *in, float *out, PlanMode plan_mode = PLAN_PATIENT);
void   fftsr_float (size_t N, float *in, float *out, PlanMode plan_mode = PLAN_PATIENT);
void   fftsr_destructive_float (size_t N, float *in, float *out, PlanMode plan_mode = PLAN_PATIENT);
void   fftac_float (size_t N, float *in, float *out, PlanMode plan_mode = PLAN_PATIENT);
void   fftsc_float (size_t N, float *in, float *out, PlanMode plan_mode = PLAN_PATIENT);

const Plan *plan_fftsr_destructive_float (size_t N, PlanMode plan_mode = PLAN_PATIENT);
void execute_fftsr_destructive_float (size_t N, float *in, float *out, const Plan *plan) noexcept SM_CLANG_NONBLOCKING;

void   debug_in_test_program (bool enabled);

void   init();
void   cleanup();

}

}

#endif
