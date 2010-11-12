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

#include "smpolyphaseinter.hh"

#include <bse/bsemathsignal.h>
#include <math.h>

using namespace SpectMorph;

using std::vector;

#define OVERSAMPLE 64
#define WIDTH      12

PolyPhaseInter*
PolyPhaseInter::the()
{
  static PolyPhaseInter *instance = 0;

  if (!instance)
    instance = new PolyPhaseInter();

  return instance;
}

static double
sig (const vector<float>& signal, int pos)
{
  if (pos >= 0 && pos < (int)signal.size())
    return signal[pos];
  return 0;
}

double
PolyPhaseInter::get_sample (const vector<float>& signal, double pos)
{
/* linear interpolation
  int ipos = pos;
  double frac = pos - ipos;
  return sig (signal, ipos) * (1 - frac) + sig (signal, ipos + 1) * frac;
*/
  int ipos = pos;
  int frac64 = (pos - ipos) * OVERSAMPLE;
  double x_frac = (pos - ipos) * OVERSAMPLE - frac64;
  double result = 0;
  int j = -WIDTH;
  int p = filter_center + j * OVERSAMPLE - frac64;
  while (p < 0)
    {
      p += OVERSAMPLE;
      j++;
    }
  if (p == 0)
    {
      result += sig (signal, ipos + j) * x[p] * (1 - x_frac);
      p += OVERSAMPLE;
      j++;
    }
  if (ipos > WIDTH && ipos + WIDTH < (int)signal.size())  // no need to check signal boundaries for each sample
    {
      while (p < (int)x.size())
        {
          double inter_x = x[p] * (1 - x_frac) + x[p - 1] * x_frac;
          result += signal[ipos + j] * inter_x;
          p += OVERSAMPLE;
          j++;
        }
    }
  else
    {
      while (p < (int)x.size())
        {
          double inter_x = x[p] * (1 - x_frac) + x[p - 1] * x_frac;
          result += sig (signal, ipos + j) * inter_x;
          p += OVERSAMPLE;
          j++;
        }
    }
  return result;
}

static double
sinc (double x)
{
  if (fabs (x) < 1e-6)
    return 1;
  else
    return sin (M_PI * x) / (M_PI * x);
}

PolyPhaseInter::PolyPhaseInter()
{
  x.resize (WIDTH * 2 * OVERSAMPLE + 1);
  filter_center = x.size() / 2;

  const double SR = 48000;
  const double LP_FREQ = 20000;
  for (int i = 0; i < (int)x.size(); i++)
    {
      int pos = i - filter_center;

      double c = sinc (double (pos) / OVERSAMPLE / (SR / 2) * LP_FREQ);
      double w = bse_window_blackman (double (pos) / filter_center) / (SR / 2) * LP_FREQ;
      x[i] = c * w;
    }
}
