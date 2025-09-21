// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MATH_HH
#define SPECTMORPH_MATH_HH

#include <math.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <smmatharm.hh>

#include "smutils.hh"

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
  uint n = 0;

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
#if defined(__SSE__) || defined(SM_ARM_SSE)
  __m128 v;   // vector of four single floats
#endif
};
/// @endcond

template<bool NEED_COS, int MODE>
inline void
internal_fast_vector_sincosf (const VectorSinParams& params, float *sin_begin, float *sin_end, float *cos_begin)
{
#if defined(__SSE__) || defined(SM_ARM_SSE)
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
  fast_vector_sincos (table_params, incf_im[0].f, (float *) &incf_im[TABLE_SIZE], incf_re[0].f);

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
      F4Vector *new_re = NEED_COS ? reinterpret_cast<F4Vector *> (cos_begin + n) : nullptr;
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
sm_ftoi (float f)
{
  int r;

  __asm__ ("fistl %0"
           : "=m" (r)
           : "t" (f));
  return r;
}
static inline int G_GNUC_CONST
sm_dtoi (double f)
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
  return int (f + 0.5f);
}
#endif

int sm_fpu_okround();

struct MathTables
{
  static float idb2f_high[256];
  static float idb2f_low[256];

  static float ifreq2f_high[256];
  static float ifreq2f_low[256];

  static float int_sincos[256];
};

#define SM_IDB_CONST_M96 uint16_t ((512 - 96) * 64)

int      sm_factor2delta_idb (double factor);
double   sm_idb2factor_slow (uint16_t idb);

void     sm_math_init();

uint16_t sm_freq2ifreq (float freq);
double   sm_ifreq2freq_slow (uint16_t ifreq);
void     sm_freq2ifreqs (float *freqs, uint n_freqs, uint16_t *out);

inline float
sm_idb2factor (uint16_t idb)
{
  return MathTables::idb2f_high[idb >> 8] * MathTables::idb2f_low[idb & 0xff];
}

inline float
sm_ifreq2freq (uint16_t ifreq)
{
  return MathTables::ifreq2f_high[ifreq >> 8] * MathTables::ifreq2f_low[ifreq & 0xff];
}

inline uint16_t
sm_factor2idb (float factor)
{
  /* 1e-25 is about the smallest factor we can properly represent as integer, as
   *
   *   20 * log10(1e-25) = 20 * -25 = -500 db
   *
   * so we map every factor that is smaller, like 0, to this value
   */
  const float db = 20 * std::log10 (std::max (factor, 1e-25f));

  return sm_round_positive (db * 64 + 512 * 64);
}

void sm_factor2idbs (float *factors, uint n_factors, uint16_t *out);

inline float
int_sinf (guint8 i)
{
  return MathTables::int_sincos[i];
}

inline float
int_cosf (guint8 i)
{
  i += 64;
  return MathTables::int_sincos[i];
}

double sm_lowpass1_factor (double mix_freq, double freq);
double sm_xparam (double x, double slope);
double sm_xparam_inv (double x, double slope);
float  sm_freq_to_note (float freq);

double sm_bessel_i0 (double x);
double velocity_to_gain (double velocity, double vrange_db);

////////////// start: code based on log2 code from Anklang/ASE by Tim Janik

/** Union to compartmentalize an IEEE-754 float.
 * IEEE 754 single precision floating point layout:
 * ```
 *        31 30           23 22            0
 * +--------+---------------+---------------+
 * | s 1bit | e[30:23] 8bit | f[22:0] 23bit |
 * +--------+---------------+---------------+
 * B0------------------->B1------->B2-->B3-->
 * ```
 */
union FloatIEEE754 {
  float         v_float;
  struct {
#if   __BYTE_ORDER == __LITTLE_ENDIAN
    uint mantissa : 23, biased_exponent : 8, sign : 1;
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint sign : 1, biased_exponent : 8, mantissa : 23;
#endif
  } mpn;
  static constexpr const int   BIAS = 127;                       ///< Exponent bias.
};

/** Fast approximation of logarithm to base 2.
 * The parameter `x` is the exponent within `[1.1e-38…2^127]`.
 * Within `1e-7…+1`, the error stays below 3.8e-6 which corresponds to a sample
 * precision of 18 bit. When `x` is an exact power of 2, the error approaches
 * zero. With FMA instructions and `-ffast-math enabled`, execution times should
 * be below 10ns on 3GHz machines.
 */

static inline float
fast_log2 (float value)
{
  const int EXPONENT_MASK = 0x7F800000;
  int iv;
  memcpy (&iv, &value, sizeof (float));                 // iv = *(int *) &values[k]
  int fexp = (iv >> 23) - FloatIEEE754::BIAS;            // extract exponent without bias (rely on sign bit == 0)
  iv = (iv & ~EXPONENT_MASK) | FloatIEEE754::BIAS << 23; // reset exponent to 2^0 so v_float is mantissa in [1..2]
  float r, x;
  memcpy (&x, &iv, sizeof (float));                      // x = *(float *) &iv
  x -= 1;
  // x=[0..1]; r = log2 (x + 1);
  // h=0.0113916; // offset to reduce error at origin
  // f=(1/log(2)) * log(x+1); dom=[0-h;1+h]; p=remez(f, 6, dom, 1);
  // p = p - p(0); // discard non-0 offset
  // err=p-f; plot(err,[0;1]); plot(f,p,dom); // result in sollya
  r = x *  -0.0259366993544709205147977455165000143561553284592936f;
  r = x * (+0.122047857676447181074792747820717519424533931189428f + r);
  r = x * (-0.27814297685064327713977752916286528359628147166014f + r);
  r = x * (+0.45764712300320092992105460899527194244236573556309f + r);
  r = x * (-0.71816105664624015087225994551041120290062342459945f + r);
  r = x * (+1.44254540258782520489769598315182363877204824648687f + r);
  return fexp + r; // log2 (i) + log2 (x)
}

/** compute fast_log2 for a block of values
 *
 * This is often faster than computing individual values, because fast_log2 and this
 * function are written in a way that both, gcc and clang should auto vectorize it
 */
extern inline void
fast_log2_block (float *values, int n_values)
{
  for (int k = 0; k < n_values; k++)
    values[k] = fast_log2 (values[k]);
}

////////////// end: code based on log2 code from Anklang/ASE by Tim Janik

} // namespace SpectMorph

#endif
