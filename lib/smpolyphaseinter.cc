// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smpolyphaseinter.hh"
#include "smmath.hh"

#include <math.h>

using namespace SpectMorph;

using std::vector;

#define OVERSAMPLE  64
#define WIDTH       7

/* set this to at least WIDTH + 2, no problem if it is a little too high */
#define MIN_PADDING 16

#include "smpolyphasecoeffs.cc"

PolyPhaseInter*
PolyPhaseInter::the()
{
  static PolyPhaseInter *instance = 0;

  if (!instance)
    instance = new PolyPhaseInter();

  return instance;
}


double
PolyPhaseInter::get_sample (const vector<float>& signal, double pos)
{
  const int ipos = pos;

  if (ipos < MIN_PADDING || ipos + MIN_PADDING > int (signal.size()))
    {
      vector<float> shift_signal (MIN_PADDING * 2);

      // shift signal: ipos should be in the center of the generated input signal
      const int shift = shift_signal.size() / 2 - ipos;

      for (int i = 0; i < int (shift_signal.size()); i++)
        {
          const int s = i - shift;

          if (s >= 0 && s < (int)signal.size())
            shift_signal[i] = signal[s];
        }

      return get_sample_no_check (shift_signal, pos + shift);
    }
  else
    {
      return get_sample_no_check (signal, pos);
    }
}

double
PolyPhaseInter::get_sample_no_check (const vector<float>& signal, double pos)
{
  const int ipos = pos;

  const int frac64 = (pos - ipos) * OVERSAMPLE;
  const float frac = (pos - ipos) * OVERSAMPLE - frac64;

  const float *x_a = &x[(WIDTH * 2 + 1) * (OVERSAMPLE - frac64)];
  const float *x_b = &x[(WIDTH * 2 + 1) * ((OVERSAMPLE * 2 - frac64 - 1) & (OVERSAMPLE - 1))];
  const float *s_ptr = &signal[ipos - WIDTH];

  float result_a = 0, result_b = 0;
  for (int j = 1; j < 2 * WIDTH + 1; j++)
    {
      result_a += s_ptr[j] * x_a[j];
      result_b += s_ptr[j] * x_b[j];
    }
  return result_a * (1 - frac) + result_b * frac;
}

size_t
PolyPhaseInter::get_min_padding()
{
  /* Minimum padding before and after the pos sample should be at least MIN_PADDING to use get_sample_no_check() */
  return MIN_PADDING;
}

PolyPhaseInter::PolyPhaseInter()
{
  vector<const double *> c ({ nullptr, c_1, c_2, c_3, c_4, c_5, c_6, c_7, c_8, c_9 });

  /*
   * reorder coefficients, first all coeffs with oversample 0, then oversample 1, ...
   */
  for (int o = 0; o <= OVERSAMPLE; o++)
    {
      int p = o & (OVERSAMPLE - 1);
      if (o != OVERSAMPLE)
        {
          x.push_back (0);
          for (int n = 0; n < WIDTH * 2; n++)
            {
              x.push_back (c_7[p]);
              p += OVERSAMPLE;
            }
        }
      else
        {
          for (int n = 0; n < WIDTH * 2 + 1; n++)
            {
              x.push_back (c_7[p]);
              p += OVERSAMPLE;
            }
        }
    }
}
