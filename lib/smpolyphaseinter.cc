// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smpolyphaseinter.hh"
#include "smmath.hh"

#include <math.h>

using namespace SpectMorph;

using std::vector;

#define OVERSAMPLE 64
#define WIDTH      7

#include "smpolyphasecoeffs.cc"

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
  // we need the biggest integer smaller than pos, so we need to handle
  // negative positions seperately
  int ipos;
  if (pos < 0)
    ipos = pos - 1.0;
  else
    ipos = pos;
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

PolyPhaseInter::PolyPhaseInter()
{
  vector<const double *> c ({ nullptr, c_1, c_2, c_3, c_4, c_5, c_6, c_7, c_8, c_9 });
  x.assign (c[WIDTH], c[WIDTH] + WIDTH * 2 * OVERSAMPLE + 1);
  filter_center = x.size() / 2;
}
