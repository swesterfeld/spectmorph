// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmath.hh"
#include <bse/bsemathsignal.hh>
#include <QtGlobal>

namespace SpectMorph {

int
sm_factor2delta_idb (double factor)
{
  return int (sm_factor2idb (factor)) - (512 * 64);
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
}

}
