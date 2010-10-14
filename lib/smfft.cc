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
#include <bse/bseblockutils.hh>
#include <string.h>

using namespace SpectMorph;
using std::map;
using std::string;

static bool enable_gsl_fft = false;

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
gsl_fftac_float (size_t N, float* in, float* out)
{
  const int ARRAY_SIZE = N * 2;
  double ind[ARRAY_SIZE];
  double outd[ARRAY_SIZE];

  std::copy (in, in + ARRAY_SIZE, ind);
  gsl_power2_fftac (N, ind, outd);
  std::copy (outd, outd + ARRAY_SIZE, out);
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

static inline void
gsl_fftsc_float (size_t N, float* in, float* out)
{
  const int ARRAY_SIZE = N * 2;
  double ind[ARRAY_SIZE];
  double outd[ARRAY_SIZE];

  std::copy (in, in + ARRAY_SIZE, ind);
  gsl_power2_fftsc (N, ind, outd);
  std::copy (outd, outd + ARRAY_SIZE, out);
}

#if SPECTMORPH_HAVE_FFTW

#include <fftw3.h>

static void save_wisdom();

float *
FFT::new_array_float (size_t N)
{
  return (float *) fftwf_malloc (sizeof (float) * (N + 2));   /* extra space for r2c extra complex output */
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
  fftwf_plan& plan = fftar_float_plan[N];

  if (!plan)
    {
      float *plan_in = new_array_float (N);
      float *plan_out = new_array_float (N);
      plan = fftwf_plan_dft_r2c_1d (N, plan_in, (fftwf_complex *) plan_out,
                                    FFTW_PATIENT | FFTW_PRESERVE_INPUT | FFTW_WISDOM_ONLY);
      if (!plan) /* missing from wisdom -> create plan and save it */
        {
          plan = fftwf_plan_dft_r2c_1d (N, plan_in, (fftwf_complex *) plan_out,
                                        FFTW_PATIENT | FFTW_PRESERVE_INPUT);
          save_wisdom();
        }
    }
  fftwf_execute_dft_r2c (plan, in, (fftwf_complex *) out);

  out[1] = out[N];
}

static map<int, fftwf_plan> fftsr_float_plan;

void
FFT::fftsr_float (size_t N, float *in, float *out)
{
  fftwf_plan& plan = fftsr_float_plan[N];

  if (!plan)
    {
      float *plan_in = new_array_float (N);
      float *plan_out = new_array_float (N);
      plan = fftwf_plan_dft_c2r_1d (N, (fftwf_complex *) plan_in, plan_out,
                                    FFTW_PATIENT | FFTW_PRESERVE_INPUT | FFTW_WISDOM_ONLY);
      if (!plan) /* missing from wisdom -> create plan and save it */
        {
          plan = fftwf_plan_dft_c2r_1d (N, (fftwf_complex *) plan_in, plan_out,
                                        FFTW_PATIENT | FFTW_PRESERVE_INPUT);
          save_wisdom();
        }
    }
  in[N] = in[1];
  in[N+1] = 0;
  in[1] = 0;

  fftwf_execute_dft_c2r (plan, (fftwf_complex *)in, out);

  in[1] = in[N]; // we need to preserve the input array
}

static map<int, fftwf_plan> fftac_float_plan;

void
FFT::fftac_float (size_t N, float *in, float *out)
{
  fftwf_plan& plan = fftac_float_plan[N];
  if (!plan)
    {
      float *plan_in = new_array_float (N * 2);
      float *plan_out = new_array_float (N * 2);

      plan = fftwf_plan_dft_1d (N, (fftwf_complex *) plan_in, (fftwf_complex *) plan_out,
                                FFTW_FORWARD, FFTW_PATIENT | FFTW_PRESERVE_INPUT | FFTW_WISDOM_ONLY);
      if (!plan) /* missing from wisdom -> create plan and save it */
        {
          plan = fftwf_plan_dft_1d (N, (fftwf_complex *) plan_in, (fftwf_complex *) plan_out,
                                    FFTW_FORWARD, FFTW_PATIENT | FFTW_PRESERVE_INPUT);
          save_wisdom();
        }
    }

  fftwf_execute_dft (plan, (fftwf_complex *)in, (fftwf_complex *)out);
}

static map<int, fftwf_plan> fftsc_float_plan;

void
FFT::fftsc_float (size_t N, float *in, float *out)
{
  fftwf_plan& plan = fftsc_float_plan[N];
  if (!plan)
    {
      float *plan_in = new_array_float (N * 2);
      float *plan_out = new_array_float (N * 2);

      plan = fftwf_plan_dft_1d (N, (fftwf_complex *) plan_in, (fftwf_complex *) plan_out,
                                FFTW_BACKWARD, FFTW_PATIENT | FFTW_PRESERVE_INPUT | FFTW_WISDOM_ONLY);
      if (!plan) /* missing from wisdom -> create plan and save it */
        {
          plan = fftwf_plan_dft_1d (N, (fftwf_complex *) plan_in, (fftwf_complex *) plan_out,
                                    FFTW_BACKWARD, FFTW_PATIENT | FFTW_PRESERVE_INPUT);
          save_wisdom();
        }
     }
  fftwf_execute_dft (plan, (fftwf_complex *)in, (fftwf_complex *)out);
}

static string
wisdom_filename()
{
  const char *homedir = g_get_home_dir();
  const char *hostname = g_get_host_name();
  return homedir + string ("/.spectmorph_fftw_wisdom_") + hostname;
}

static void
save_wisdom()
{
  /* detect if we're running in valgrind - in this case newly accumulated wisdom is probably flawed */
  bool valgrind = false;

  FILE *maps = fopen (Birnet::string_printf ("/proc/%d/maps", getpid()).c_str(), "r");
  if (maps)
    {
      char buffer[1024];
      while (fgets (buffer, 1024, maps))
        {
          if (strstr (buffer, "vgpreload"))
            valgrind = true;
        }
      fclose (maps);
    }
  if (valgrind)
    {
      printf ("FFT::save_wisdom(): not saving fft wisdom (running under valgrind)\n");
      return;
    }
  /* atomically replace old wisdom file with new wisdom file
   *
   * its theoretically possible (but highly unlikely) that we leak a *wisdom*.new.12345 file
   */
  string new_wisdom_filename = Birnet::string_printf ("%s.new.%d", wisdom_filename().c_str(), getpid());
  FILE *outfile = fopen (new_wisdom_filename.c_str(), "w");
  if (outfile)
    {
      fftwf_export_wisdom_to_file (outfile);
      fclose (outfile);
      rename (new_wisdom_filename.c_str(), wisdom_filename().c_str());
    }
}

void
FFT::load_wisdom()
{
  FILE *infile = fopen (wisdom_filename().c_str(), "r");
  if (infile)
    {
      fftwf_import_wisdom_from_file (infile);
      fclose (infile);
    }
}

#else

#error "building without FFTW is not supported currently"

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
  gsl_fftsr_float (N, in, out);
}

void
FFT::fftac_float (size_t N, float *in, float *out)
{
  gsl_fftac_float (N, in, out);
}

void
FFT::fftsc_float (size_t N, float *in, float *out)
{
  gsl_fftsc_float (N, in, out);
}

void
FFT::load_wisdom()
{
  /* no FFTW */
}

#endif
