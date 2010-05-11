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

namespace SpectMorph
{

struct
VectorSinParams
{
  VectorSinParams() :
    mix_freq (-1),
    freq (-1),
    phase (-100),
    mag (-1)
  {
  }
  double mix_freq;
  double freq;
  double phase;
  double mag;
};


template<class Iterator>
void
fast_vector_sin_add (const VectorSinParams& params, Iterator begin, Iterator end)
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

} // namespace SpectMorph

#endif
