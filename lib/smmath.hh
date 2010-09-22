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

#ifndef SPECTMORPH_MATH_HH
#define SPECTMORPH_MATH_HH

#include <math.h>
#include <glib.h>
#include <string.h>
#ifdef __SSE__
#include <xmmintrin.h>
#endif

namespace SpectMorph
{

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

  sincos (params.phase, &state_im, &state_re);
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
          sincos (phase_inc * n + params.phase, &state_im, &state_re);
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

  sincos (params.phase, &state_im, &state_re);
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
          sincos (phase_inc * n + params.phase, &state_im, &state_re);
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

  sincos (params.phase, &state_im, &state_re);
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

inline void
int_sincos (guint32 i, double *si, double *ci)
{
  extern float *int_sincos_table;

  i &= 0xff;

  *si = int_sincos_table[i];
  *ci = int_sincos_table[(i + 64) & 0xff];
}

inline void
int_sincos_init()
{
  extern float *int_sincos_table;

  int_sincos_table = (float *) malloc (sizeof (float) * 256);
  for (int i = 0; i < 256; i++)
    int_sincos_table[i] = sin (double (i / 256.0) * 2 * M_PI);
}

} // namespace SpectMorph

#endif
