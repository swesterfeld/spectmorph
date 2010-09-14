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

#include "smfft.hh"
#include <algorithm>
#include <map>
#include "config.h"

#include <glib.h>
#include <bse/gslfft.h>

using namespace SpectMorph;
using std::map;

static bool enable_gsl_fft = true;

void
FFT::use_gsl_fft (bool new_enable_gsl_fft)
{
  enable_gsl_fft = new_enable_gsl_fft;
}

static inline void
gsl_fftar_float (size_t N, float* in, float* out)
{
  double ind[N];
  double outd[N];

  std::copy (in, in + N, ind);
  gsl_power2_fftar (N, ind, outd);
  std::copy (outd, outd + N, out);
}

static inline void
gsl_fftsr_float (size_t N, float* in, float* out)
{
  double ind[N];
  double outd[N];

  std::copy (in, in + N, ind);
  gsl_power2_fftsr (N, ind, outd);
  std::copy (outd, outd + N, out);
}

#if SPECTMORPH_HAVE_FFTW

#include <fftw3.h>

float *
FFT::new_array_float (size_t N)
{
  return (float *) fftwf_malloc (sizeof (float) * N + 2);   /* extra space for r2c extra complex output */
}

void
FFT::free_array_float (float *f)
{
  fftwf_free (f);
}

static map<int, fftwf_plan> fftar_float_plan;

void
FFT::fftar_float (size_t N, float *in, float *out)
{
  if (enable_gsl_fft)
    {
      gsl_fftar_float (N, in, out);
      return;
    }

  fftwf_plan& plan = fftar_float_plan[N];

  if (!plan)
    {
      float *plan_in = new_array_float (N);
      float *plan_out = new_array_float (N);
      plan = fftwf_plan_dft_r2c_1d (N, plan_in, (fftwf_complex *) plan_out, FFTW_PATIENT);
    }
  fftwf_execute_dft_r2c (plan, in, (fftwf_complex *) out);
  out[1] = out[N];
}

static map<int, fftwf_plan> fftsr_float_plan;

void
FFT::fftsr_float (size_t N, float *in, float *out)
{
  if (enable_gsl_fft)
    {
      gsl_fftsr_float (N, in, out);
      return;
    }

  fftwf_plan& plan = fftsr_float_plan[N];

  if (!plan)
    {
      float *plan_in = new_array_float (N);
      float *plan_out = new_array_float (N);
      plan = fftwf_plan_dft_c2r_1d (N, (fftwf_complex *) plan_in, plan_out, FFTW_PATIENT);
    }
  in[N] = in[1];
  in[N+1] = 0;
  in[1] = 0;
  fftwf_execute_dft_c2r (plan, (fftwf_complex *)in, out);
  const double scale = 1.0 / N;
  for (size_t i = 0; i < N; i++)
    out[i] *= scale;
}

#else

float *
FFT::new_array_float (size_t N)
{
  return (float *) g_malloc (N * sizeof (float));
}

void
FFT::free_array_float (float *f)
{
  g_free (f);
}

void
FFT::fftar_float (size_t N, float* in, float* out)
{
  gsl_fftar_float (N, in, out);
}

void
FFT::fftsr_float (size_t N, float* in, float* out)
{
  gsl_fftsr_float (N, in, out)
}

#endif
