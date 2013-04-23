// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmath.hh"
#include <bse/bsemathsignal.hh>
#include <QtGlobal>

namespace SpectMorph {

uint16_t
sm_factor2idb (double factor)
{
  double db = bse_db_from_factor (factor, -500);

  return sm_round_positive (db * 64 + 512 * 64);
}

double
sm_idb2factor_slow (uint16_t idb)
{
  double db = idb / 64.0 - 512;
  return bse_db_to_factor (db);
}

#define FAC 6000.0
#define ADD (3 * FAC)

uint16_t
sm_freq2ifreq (double freq)
{
  return qBound (0, sm_round_positive (log (freq) * FAC + ADD), 65535);
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
static float idb2f_high[256];
static float idb2f_low[256];

static float ifreq2f_high[256];
static float ifreq2f_low[256];

double
sm_idb2factor (uint16_t idb)
{
  return idb2f_high[idb >> 8] * idb2f_low[idb & 0xff];
}

double
sm_ifreq2freq (uint16_t ifreq)
{
  return ifreq2f_high[ifreq >> 8] * ifreq2f_low[ifreq & 0xff];
}

void
sm_math_init()
{
  for (size_t i = 0; i < 256; i++)
    {
      idb2f_high[i] = sm_idb2factor_slow (i * 256);
      idb2f_low[i]  = sm_idb2factor_slow (64 * 512 + i);

      ifreq2f_high[i] = sm_ifreq2freq_slow (i * 256);
      ifreq2f_low[i]  = sm_ifreq2freq_slow (ADD + i);
    }
}

}
