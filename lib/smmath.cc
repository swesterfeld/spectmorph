// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmath.hh"
#include <assert.h>

namespace SpectMorph {

double
db_to_factor (double dB)
{
  double factor = dB / 20; /* Bell */
  return pow (10, factor);
}

double
db_from_factor (double factor, double min_dB)
{
  if (factor > 0)
    {
      double dB = log10 (factor); /* Bell */
      dB *= 20;
      return dB;
    }
  else
    return min_dB;
}

int
sm_factor2delta_idb (double factor)
{
  return int (sm_factor2idb (factor)) - (512 * 64);
}

double
sm_idb2factor_slow (uint16_t idb)
{
  double db = idb / 64.0 - 512;
  return db_to_factor (db);
}

#define FAC 6000.0
#define ADD (3 * FAC)

uint16_t
sm_freq2ifreq (double freq)
{
  return sm_bound (0, sm_round_positive (log (freq) * FAC + ADD), 65535);
}

double
sm_ifreq2freq_slow (uint16_t ifreq)
{
  return exp ((ifreq - ADD) / FAC);
}

/* tables for:
 *
 *  - fast idb -> factor conversion
 *  - fast ifreq -> freq conversion
 *
 * exp (high + low) = exp (high) * exp (low)
 */
float MathTables::idb2f_high[256];
float MathTables::idb2f_low[256];

float MathTables::ifreq2f_high[256];
float MathTables::ifreq2f_low[256];

void
sm_math_init()
{
  for (size_t i = 0; i < 256; i++)
    {
      MathTables::idb2f_high[i] = sm_idb2factor_slow (i * 256);
      MathTables::idb2f_low[i]  = sm_idb2factor_slow (64 * 512 + i);

      MathTables::ifreq2f_high[i] = sm_ifreq2freq_slow (i * 256);
      MathTables::ifreq2f_low[i]  = sm_ifreq2freq_slow (ADD + i);
    }

#if defined (__i386__) && defined (__GNUC__)
  // ensure proper rounding mode
  assert (sm_fpu_okround());
#endif

  assert (sm_round_positive (42.51) == 43);
  assert (sm_round_positive (3.14) == 3);
  assert (sm_round_positive (2.1) == 2);
  assert (sm_round_positive (0.7) == 1);
  assert (sm_round_positive (0.2) == 0);
}

#if defined (__i386__) && defined (__GNUC__)
int
sm_fpu_okround()
{
  typedef unsigned short int BseFpuState;

  BseFpuState cv;
  __asm__ ("fnstcw %0"
           : "=m" (*&cv));
  return !(cv & 0x0c00);
}
#endif

float
sm_freq_to_note (float freq)
{
  return 69 + 12 * std::log2 (freq / 440);
}

double
sm_lowpass1_factor (double mix_freq, double freq)
{
  const double delta_t = (1 / mix_freq);

  return 2 * M_PI * delta_t * freq / (2 * M_PI * delta_t * freq + 1);
}

double
sm_xparam (double x, double slope)
{
  return pow (x, slope);
}

double
sm_xparam_inv (double x, double slope)
{
  return pow (x, 1 / slope);
}

double
sm_bessel_i0 (double x)
{
  /* http://www.vibrationdata.com/Bessel.htm */

  /* 1 + (x/2)^2/(1!^2)
   *   + (x/2)^4/(2!^2)
   *   + (x/2)^6/(3!^2)   ... */

  double delta = 1;
  double result = 1;
  const double sqr_x_2 = (x/2)*(x/2);

  for (int i = 1; i < 500; i++)
    {
      delta *= sqr_x_2 / (i * i);
      result += delta;

      if (delta < 1e-14 * result)
        break;
    }
  return result;
}

double
velocity_to_gain (double velocity, double vrange_db)
{
  /* vrange_db should be positive or zero */
  g_return_val_if_fail (vrange_db > -0.01, 0);

  /* convert, so that
   *  - gain (0)   is   at the -vrange_db
   *  - gain (1)   is   0 db
   *  - sqrt(gain(v)) is a straight line
   *
   *  See Roger B. Dannenberg: The Interpretation of Midi Velocity
   */
  const double b = db_to_factor (-vrange_db * 0.5);
  const double x = (1 - b) * velocity + b;

  return x * x;
}

}
