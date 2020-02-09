// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smfft.hh"
#include "smutils.hh"
#include <algorithm>
#include <map>
#include <mutex>
#include "config.h"

#include <glib.h>
#include <string.h>
#include <unistd.h>

using namespace SpectMorph;
using std::map;
using std::string;

static bool enable_gsl_fft = false;
static bool randomize_new_fft_arrays = false;

void
FFT::use_gsl_fft (bool new_enable_gsl_fft)
{
  enable_gsl_fft = new_enable_gsl_fft;
}

void
FFT::debug_randomize_new_arrays (bool b)
{
  randomize_new_fft_arrays = b;
}

#if SPECTMORPH_HAVE_FFTW

#include <fftw3.h>

static void save_wisdom();

/*
 * we force that only one thread at a time will be in planning mode
 *  - fftw planner is not threadsafe
 *  - neither is our code that generates plans
 */
static std::mutex fftw_plan_mutex;
static std::mutex plan_map_mutex;

static fftwf_plan&
read_plan_map_threadsafe (std::map<int, fftwf_plan>& plan_map, size_t N)
{
  /* std::map access is not threadsafe */
  std::lock_guard<std::mutex> lg (plan_map_mutex);
  return plan_map[N];
}

float *
FFT::new_array_float (size_t N)
{
  const size_t N_2 = N + 2; /* extra space for r2c extra complex output */

  float *result = (float *) fftwf_malloc (sizeof (float) * N_2);

  if (randomize_new_fft_arrays)
    {
      for (size_t i = 0; i < N_2; i++)
        result[i] = g_random_double_range (-1, 1);
    }
  return result;
}

void
FFT::free_array_float (float *f)
{
  fftwf_free (f);
}

static map<int, fftwf_plan> fftar_float_plan;

static int
plan_flags (FFT::PlanMode plan_mode)
{
  switch (plan_mode)
    {
    case FFT::PLAN_PATIENT:    return (FFTW_PATIENT | FFTW_PRESERVE_INPUT | FFTW_WISDOM_ONLY);
    case FFT::PLAN_ESTIMATE:   return (FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
    default:                   g_assert_not_reached();
    }
}

void
FFT::fftar_float (size_t N, float *in, float *out, PlanMode plan_mode)
{
  fftwf_plan& plan = read_plan_map_threadsafe (fftar_float_plan, N);

  if (!plan)
    {
      std::lock_guard<std::mutex> lg (fftw_plan_mutex);
      float *plan_in = new_array_float (N);
      float *plan_out = new_array_float (N);
      plan = fftwf_plan_dft_r2c_1d (N, plan_in, (fftwf_complex *) plan_out, plan_flags (plan_mode));
      if (!plan) /* missing from wisdom -> create plan and save it */
        {
          plan = fftwf_plan_dft_r2c_1d (N, plan_in, (fftwf_complex *) plan_out, plan_flags (plan_mode) & ~FFTW_WISDOM_ONLY);
          save_wisdom();
        }
      free_array_float (plan_out);
      free_array_float (plan_in);
    }
  fftwf_execute_dft_r2c (plan, in, (fftwf_complex *) out);

  out[1] = out[N];
}

static map<int, fftwf_plan> fftsr_float_plan;

void
FFT::fftsr_float (size_t N, float *in, float *out, PlanMode plan_mode)
{
  fftwf_plan& plan = read_plan_map_threadsafe (fftsr_float_plan, N);

  if (!plan)
    {
      std::lock_guard<std::mutex> lg (fftw_plan_mutex);
      float *plan_in = new_array_float (N);
      float *plan_out = new_array_float (N);
      plan = fftwf_plan_dft_c2r_1d (N, (fftwf_complex *) plan_in, plan_out, plan_flags (plan_mode));
      if (!plan) /* missing from wisdom -> create plan and save it */
        {
          plan = fftwf_plan_dft_c2r_1d (N, (fftwf_complex *) plan_in, plan_out, plan_flags (plan_mode) & ~FFTW_WISDOM_ONLY);
          save_wisdom();
        }
      free_array_float (plan_out);
      free_array_float (plan_in);
    }
  in[N] = in[1];
  in[N+1] = 0;
  in[1] = 0;

  fftwf_execute_dft_c2r (plan, (fftwf_complex *)in, out);

  in[1] = in[N]; // we need to preserve the input array
}

static map<int, fftwf_plan> fftsr_destructive_float_plan;

void
FFT::fftsr_destructive_float (size_t N, float *in, float *out, PlanMode plan_mode)
{
  fftwf_plan& plan = read_plan_map_threadsafe (fftsr_destructive_float_plan, N);

  if (!plan)
    {
      std::lock_guard<std::mutex> lg (fftw_plan_mutex);
      int xplan_flags = plan_flags (plan_mode) & ~FFTW_PRESERVE_INPUT;
      float *plan_in = new_array_float (N);
      float *plan_out = new_array_float (N);
      plan = fftwf_plan_dft_c2r_1d (N, (fftwf_complex *) plan_in, plan_out, xplan_flags);
      if (!plan) /* missing from wisdom -> create plan and save it */
        {
          plan = fftwf_plan_dft_c2r_1d (N, (fftwf_complex *) plan_in, plan_out,
                                        xplan_flags & ~FFTW_WISDOM_ONLY);
          save_wisdom();
        }
      free_array_float (plan_out);
      free_array_float (plan_in);
    }
  in[N] = in[1];
  in[N+1] = 0;
  in[1] = 0;

  fftwf_execute_dft_c2r (plan, (fftwf_complex *)in, out);
}

static map<int, fftwf_plan> fftac_float_plan;

void
FFT::fftac_float (size_t N, float *in, float *out, PlanMode plan_mode)
{
  fftwf_plan& plan = read_plan_map_threadsafe (fftac_float_plan, N);
  if (!plan)
    {
      std::lock_guard<std::mutex> lg (fftw_plan_mutex);
      float *plan_in = new_array_float (N * 2);
      float *plan_out = new_array_float (N * 2);

      plan = fftwf_plan_dft_1d (N, (fftwf_complex *) plan_in, (fftwf_complex *) plan_out,
                                FFTW_FORWARD, plan_flags (plan_mode));
      if (!plan) /* missing from wisdom -> create plan and save it */
        {
          plan = fftwf_plan_dft_1d (N, (fftwf_complex *) plan_in, (fftwf_complex *) plan_out,
                                    FFTW_FORWARD, plan_flags (plan_mode) & ~FFTW_WISDOM_ONLY);
          save_wisdom();
        }
      free_array_float (plan_out);
      free_array_float (plan_in);
    }

  fftwf_execute_dft (plan, (fftwf_complex *)in, (fftwf_complex *)out);
}

static map<int, fftwf_plan> fftsc_float_plan;

void
FFT::fftsc_float (size_t N, float *in, float *out, PlanMode plan_mode)
{
  fftwf_plan& plan = read_plan_map_threadsafe (fftsc_float_plan, N);
  if (!plan)
    {
      std::lock_guard<std::mutex> lg (fftw_plan_mutex);
      float *plan_in = new_array_float (N * 2);
      float *plan_out = new_array_float (N * 2);

      plan = fftwf_plan_dft_1d (N, (fftwf_complex *) plan_in, (fftwf_complex *) plan_out,
                                FFTW_BACKWARD, plan_flags (plan_mode));
      if (!plan) /* missing from wisdom -> create plan and save it */
        {
          plan = fftwf_plan_dft_1d (N, (fftwf_complex *) plan_in, (fftwf_complex *) plan_out,
                                    FFTW_BACKWARD, plan_flags (plan_mode) & ~FFTW_WISDOM_ONLY);
          save_wisdom();
        }
      free_array_float (plan_out);
      free_array_float (plan_in);
    }
  fftwf_execute_dft (plan, (fftwf_complex *)in, (fftwf_complex *)out);
}

static string
wisdom_filename()
{
  const char *hostname = g_get_host_name();
  return sm_get_user_dir (USER_DIR_DATA) + string ("/.fftw_wisdom_") + hostname;
}

static void
save_wisdom()
{
  /* detect if we're running in valgrind - in this case newly accumulated wisdom is probably flawed */
  bool valgrind = false;

  FILE *maps = fopen (string_printf ("/proc/%d/maps", getpid()).c_str(), "r");
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
  string new_wisdom_filename = string_printf ("%s.new.%d", wisdom_filename().c_str(), getpid());
  FILE *outfile = fopen (new_wisdom_filename.c_str(), "w");
  if (outfile)
    {
      fftwf_export_wisdom_to_file (outfile);
      fclose (outfile);
      rename (new_wisdom_filename.c_str(), wisdom_filename().c_str());
    }
}

static void
load_wisdom()
{
  FILE *infile = fopen (wisdom_filename().c_str(), "r");
  if (infile)
    {
      fftwf_import_wisdom_from_file (infile);
      fclose (infile);
    }
}

void
FFT::init()
{
  load_wisdom();
}

void
FFT::cleanup()
{
  typedef map<int, fftwf_plan> PlanMap;

  auto cleanup_plans = [](PlanMap& plan_map) {
    for (auto& plan_entry : plan_map)
      fftwf_destroy_plan (plan_entry.second);

    plan_map.clear();
  };
  cleanup_plans (fftar_float_plan);
  cleanup_plans (fftsr_float_plan);
  cleanup_plans (fftsr_destructive_float_plan);
  cleanup_plans (fftac_float_plan);
  cleanup_plans (fftsc_float_plan);
}

#else

#error "building without FFTW is not supported currently"

#endif
