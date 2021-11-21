// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MATH_HH
#define SPECTMORPH_MATH_HH

#include <math.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <smmatharm.hh>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#include <algorithm>
#include <cmath>

namespace SpectMorph
{

/* Unfortunately, if we just write fabs(x) in our code, the return value type
 * appears to be compiler/version dependant:
 *  - if x is a double, the result is a double (just as in plain C)
 *  - if x is a float
 *    - some compilers return double (C style)
 *    - some compilers return float (C++ style)
 *
 * This "using" should enforce C++ style behaviour for fabs(x) for all compilers,
 * as long as we do using namespace SpectMorph (which we should always do).
 */
using std::fabs;

inline void
sm_sincos (double x, double *s, double *c)
{
#if __APPLE__
  *s = sin (x); // sincos is a gnu extension
  *c = cos (x);
#else
  sincos (x, s, c);
#endif
}


/**
 * \brief parameter structure for the various optimized vector sine functions
 */
struct
VectorSinParams
{
  double mix_freq;         //!< the mix freq (sampling rate) of the sin (and cos) wave to be created
  double freq;             //!< the frequency of the sin (and cos) wave to be created
  double phase;            //!< the start phase of the wave
  double mag;              //!< the magnitude (amplitude) of the wave

  enum {
    NONE = -1,
    ADD  = 1,              //!< add computed values to the values that are in the output array
    REPLACE = 2            //!< replace values in the output array with computed values 
  } mode;                  //!< whether to overwrite or add output

  VectorSinParams() :
    mix_freq (-1),
    freq (-1),
    phase (-100),
    mag (-1),
    mode (NONE)
  {
  }
};

template<class Iterator, int MODE>
inline void
internal_fast_vector_sin (const VectorSinParams& params, Iterator begin, Iterator end)
{
  g_return_if_fail (params.mix_freq > 0 && params.freq > 0 && params.phase > -99 && params.mag > 0);

  const double phase_inc = params.freq / params.mix_freq * 2 * M_PI;
  const double inc_re = cos (phase_inc);
  const double inc_im = sin (phase_inc);
  int n = 0;

  double state_re;
  double state_im;

  sm_sincos (params.phase, &state_im, &state_re);
  state_re *= params.mag;
  state_im *= params.mag;

  for (Iterator x = begin; x != end; x++)
    {
      if (MODE == VectorSinParams::REPLACE)
        *x = state_im;
      else
        *x += state_im;
      if ((n++ & 255) == 255)
        {
          sm_sincos (phase_inc * n + params.phase, &state_im, &state_re);
          state_re *= params.mag;
          state_im *= params.mag;
        }
      else
        {
          /*
           * (state_re + i * state_im) * (inc_re + i * inc_im) =
           *   state_re * inc_re - state_im * inc_im + i * (state_re * inc_im + state_im * inc_re)
           */
          const double re = state_re * inc_re - state_im * inc_im;
          const double im = state_re * inc_im + state_im * inc_re;
          state_re = re;
          state_im = im;
        }
    }
}

template<class Iterator, int MODE>
inline void
internal_fast_vector_sincos (const VectorSinParams& params, Iterator sin_begin, Iterator sin_end, Iterator cos_begin)
{
  g_return_if_fail (params.mix_freq > 0 && params.freq > 0 && params.phase > -99 && params.mag > 0);

  const double phase_inc = params.freq / params.mix_freq * 2 * M_PI;
  const double inc_re = cos (phase_inc);
  const double inc_im = sin (phase_inc);
  int n = 0;

  double state_re;
  double state_im;

  sm_sincos (params.phase, &state_im, &state_re);
  state_re *= params.mag;
  state_im *= params.mag;

  for (Iterator x = sin_begin, y = cos_begin; x != sin_end; x++, y++)
    {
      if (MODE == VectorSinParams::REPLACE)
        {
          *x = state_im;
          *y = state_re;
        }
      else
        {
          *x += state_im;
          *y += state_re;
        }
      if ((n++ & 255) == 255)
        {
          sm_sincos (phase_inc * n + params.phase, &state_im, &state_re);
          state_re *= params.mag;
          state_im *= params.mag;
        }
      else
        {
          /*
           * (state_re + i * state_im) * (inc_re + i * inc_im) =
           *   state_re * inc_re - state_im * inc_im + i * (state_re * inc_im + state_im * inc_re)
           */
          const double re = state_re * inc_re - state_im * inc_im;
          const double im = state_re * inc_im + state_im * inc_re;
          state_re = re;
          state_im = im;
        }
    }
}

template<class Iterator>
inline void
fast_vector_sin (const VectorSinParams& params, Iterator sin_begin, Iterator sin_end)
{
  if (params.mode == VectorSinParams::ADD)
    {
      internal_fast_vector_sin<Iterator, VectorSinParams::ADD> (params, sin_begin, sin_end);
    }
  else if (params.mode == VectorSinParams::REPLACE)
    {
      internal_fast_vector_sin<Iterator, VectorSinParams::REPLACE> (params, sin_begin, sin_end);
    }
  else
    {
      g_assert_not_reached();
    }
}

template<class Iterator>
inline void
fast_vector_sincos (const VectorSinParams& params, Iterator sin_begin, Iterator sin_end, Iterator cos_begin)
{
  if (params.mode == VectorSinParams::ADD)
    {
      internal_fast_vector_sincos<Iterator, VectorSinParams::ADD> (params, sin_begin, sin_end, cos_begin);
    }
  else if (params.mode == VectorSinParams::REPLACE)
    {
      internal_fast_vector_sincos<Iterator, VectorSinParams::REPLACE> (params, sin_begin, sin_end, cos_begin);
    }
  else
    {
      g_assert_not_reached();
    }
}


/// @cond
/* see: http://ds9a.nl/gcc-simd/ */
union F4Vector
{
  float f[4];
#ifdef __SSE__
  __m128 v;   // vector of four single floats
#endif
};
/// @endcond

template<bool NEED_COS, int MODE>
inline void
internal_fast_vector_sincosf (const VectorSinParams& params, float *sin_begin, float *sin_end, float *cos_begin)
{
#ifdef __SSE__
  g_return_if_fail (params.mix_freq > 0 && params.freq > 0 && params.phase > -99 && params.mag > 0);

  const int TABLE_SIZE = 32;

  const double phase_inc = params.freq / params.mix_freq * 2 * M_PI;
  const double inc_re16 = cos (phase_inc * TABLE_SIZE * 4);
  const double inc_im16 = sin (phase_inc * TABLE_SIZE * 4);
  int n = 0;

  double state_re;
  double state_im;

  sm_sincos (params.phase, &state_im, &state_re);
  state_re *= params.mag;
  state_im *= params.mag;

  F4Vector incf_re[TABLE_SIZE];
  F4Vector incf_im[TABLE_SIZE];

  // compute tables using FPU
  VectorSinParams table_params = params;
  table_params.phase = 0;
  table_params.mag = 1;
  table_params.mode = VectorSinParams::REPLACE;
  fast_vector_sincos (table_params, incf_im[0].f, incf_im[0].f + (TABLE_SIZE * 4), incf_re[0].f);

  // inner loop using SSE instructions
  int todo = sin_end - sin_begin;
  while (todo >= 4 * TABLE_SIZE)
    {
      F4Vector sf_re;
      F4Vector sf_im;
      sf_re.f[0] = state_re;
      sf_re.f[1] = state_re;
      sf_re.f[2] = state_re;
      sf_re.f[3] = state_re;
      sf_im.f[0] = state_im;
      sf_im.f[1] = state_im;
      sf_im.f[2] = state_im;
      sf_im.f[3] = state_im;

      /*
       * formula for complex multiplication:
       *
       * (state_re + i * state_im) * (inc_re + i * inc_im) =
       *   state_re * inc_re - state_im * inc_im + i * (state_re * inc_im + state_im * inc_re)
       */
      F4Vector *new_im = reinterpret_cast<F4Vector *> (sin_begin + n);
      F4Vector *new_re = reinterpret_cast<F4Vector *> (cos_begin + n);
      for (int k = 0; k < TABLE_SIZE; k++)
        {
          if (MODE == VectorSinParams::ADD)
            {
              if (NEED_COS)
                {
                  new_re[k].v = _mm_add_ps (new_re[k].v, _mm_sub_ps (_mm_mul_ps (sf_re.v, incf_re[k].v),
                                                                     _mm_mul_ps (sf_im.v, incf_im[k].v)));
                }
              new_im[k].v = _mm_add_ps (new_im[k].v, _mm_add_ps (_mm_mul_ps (sf_re.v, incf_im[k].v),
                                                     _mm_mul_ps (sf_im.v, incf_re[k].v)));
            }
          else
            {
              if (NEED_COS)
                {
                  new_re[k].v = _mm_sub_ps (_mm_mul_ps (sf_re.v, incf_re[k].v),
                                            _mm_mul_ps (sf_im.v, incf_im[k].v));
                }
              new_im[k].v = _mm_add_ps (_mm_mul_ps (sf_re.v, incf_im[k].v),
                                        _mm_mul_ps (sf_im.v, incf_re[k].v));
            }
        }

      n += 4 * TABLE_SIZE;

      /*
       * (state_re + i * state_im) * (inc_re + i * inc_im) =
       *   state_re * inc_re - state_im * inc_im + i * (state_re * inc_im + state_im * inc_re)
       */
      const double re = state_re * inc_re16 - state_im * inc_im16;
      const double im = state_re * inc_im16 + state_im * inc_re16;
      state_re = re;
      state_im = im;

      todo -= 4 * TABLE_SIZE;
    }

  // compute the remaining sin/cos values using the FPU
  VectorSinParams rest_params = params;
  rest_params.phase += n * phase_inc;
  if (NEED_COS)
    fast_vector_sincos (rest_params, sin_begin + n, sin_end, cos_begin + n);
  else
    fast_vector_sin (rest_params, sin_begin + n, sin_end);
#else
  if (NEED_COS)
    fast_vector_sincos (params, sin_begin, sin_end, cos_begin);
  else
    fast_vector_sin (params, sin_begin, sin_end);
#endif
}

inline void
fast_vector_sincosf (const VectorSinParams& params, float *sin_begin, float *sin_end, float *cos_begin)
{
  if (params.mode == VectorSinParams::ADD)
    {
      internal_fast_vector_sincosf<true, VectorSinParams::ADD> (params, sin_begin, sin_end, cos_begin);
    }
  else if (params.mode == VectorSinParams::REPLACE)
    {
      internal_fast_vector_sincosf<true, VectorSinParams::REPLACE> (params, sin_begin, sin_end, cos_begin);
    }
  else
    {
      g_assert_not_reached();
    }
}

inline void
fast_vector_sinf (const VectorSinParams& params, float *sin_begin, float *sin_end)
{
  if (params.mode == VectorSinParams::ADD)
    {
      internal_fast_vector_sincosf<false, VectorSinParams::ADD> (params, sin_begin, sin_end, NULL);
    }
  else if (params.mode == VectorSinParams::REPLACE)
    {
      internal_fast_vector_sincosf<false, VectorSinParams::REPLACE> (params, sin_begin, sin_end, NULL);
    }
  else
    {
      g_assert_not_reached();
    }
}

inline void
zero_float_block (size_t n_values, float *values)
{
  memset (values, 0, n_values * sizeof (float));
}

inline float
int_sinf (guint8 i)
{
  extern float *int_sincos_table;

  return int_sincos_table[i];
}

inline float
int_cosf (guint8 i)
{
  extern float *int_sincos_table;

  i += 64;
  return int_sincos_table[i];
}

inline void
int_sincos_init()
{
  extern float *int_sincos_table;

  int_sincos_table = (float *) malloc (sizeof (float) * 256);
  for (int i = 0; i < 256; i++)
    int_sincos_table[i] = sin (double (i / 256.0) * 2 * M_PI);
}

/* --- signal processing: windows --- */

inline double
window_cos (double x) /* von Hann window */
{
  if (fabs (x) > 1)
    return 0;
  return 0.5 * cos (x * M_PI) + 0.5;
}

inline double
window_hamming (double x) /* sharp (rectangle) cutoffs at boundaries */
{
  if (fabs (x) > 1)
    return 0;

  return 0.54 + 0.46 * cos (M_PI * x);
}

inline double
window_blackman (double x)
{
  if (fabs (x) > 1)
    return 0;
  return 0.42 + 0.5 * cos (M_PI * x) + 0.08 * cos (2.0 * M_PI * x);
}

inline double
window_blackman_harris_92 (double x)
{
  if (fabs (x) > 1)
    return 0;

  const double a0 = 0.35875, a1 = 0.48829, a2 = 0.14128, a3 = 0.01168;

  return a0 + a1 * cos (M_PI * x) + a2 * cos (2.0 * M_PI * x) + a3 * cos (3.0 * M_PI * x);
}

/* --- decibel conversion --- */
double db_to_factor (double dB);
double db_from_factor (double factor, double min_dB);

#if defined (__i386__) && defined (__GNUC__)
static inline int G_GNUC_CONST
sm_ftoi (register float f)
{
  int r;

  __asm__ ("fistl %0"
           : "=m" (r)
           : "t" (f));
  return r;
}
static inline int G_GNUC_CONST
sm_dtoi (register double f)
{
  int r;

  __asm__ ("fistl %0"
           : "=m" (r)
           : "t" (f));
  return r;
}
inline int
sm_round_positive (double d)
{
  return sm_dtoi (d);
}

inline int
sm_round_positive (float f)
{
  return sm_ftoi (f);
}
#else
inline int
sm_round_positive (double d)
{
  return int (d + 0.5);
}

inline int
sm_round_positive (float f)
{
  return int (f + 0.5);
}
#endif

int sm_fpu_okround();

struct MathTables
{
  static float idb2f_high[256];
  static float idb2f_low[256];

  static float ifreq2f_high[256];
  static float ifreq2f_low[256];
};

#define SM_IDB_CONST_M96 uint16_t ((512 - 96) * 64)

int      sm_factor2delta_idb (double factor);
double   sm_idb2factor_slow (uint16_t idb);

void     sm_math_init();

uint16_t sm_freq2ifreq (double freq);
double   sm_ifreq2freq_slow (uint16_t ifreq);

inline double
sm_idb2factor (uint16_t idb)
{
  return MathTables::idb2f_high[idb >> 8] * MathTables::idb2f_low[idb & 0xff];
}

inline double
sm_ifreq2freq (uint16_t ifreq)
{
  return MathTables::ifreq2f_high[ifreq >> 8] * MathTables::ifreq2f_low[ifreq & 0xff];
}

inline uint16_t
sm_factor2idb (double factor)
{
  /* 1e-25 is about the smallest factor we can properly represent as integer, as
   *
   *   20 * log10(1e-25) = 20 * -25 = -500 db
   *
   * so we map every factor that is smaller, like 0, to this value
   */
  const double db = 20 * log10 (std::max (factor, 1e-25));

  return sm_round_positive (db * 64 + 512 * 64);
}

double sm_lowpass1_factor (double mix_freq, double freq);
double sm_xparam (double x, double slope);
double sm_xparam_inv (double x, double slope);

double sm_bessel_i0 (double x);
double velocity_to_gain (double velocity, double vrange_db);

/* FIXME: FILTER: get rid of sm_bound */
template<typename T>
inline const T&
sm_bound (const T& min_value, const T& value, const T& max_value)
{
  return std::min (std::max (value, min_value), max_value);
}

template<typename T>
inline const T&
sm_clamp (const T& value, const T& min_value, const T& max_value)
{
  return std::min (std::max (value, min_value), max_value);
}

} // namespace SpectMorph

#endif
