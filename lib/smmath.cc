// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmath.hh"
#include <bse/bsemathsignal.hh>

namespace SpectMorph {

uint16_t
sm_factor2idb (double factor)
{
  double db = bse_db_from_factor (factor, -500);

  return sm_round_positive (db * 64 + 512 * 64);
}

double
sm_idb2factor (uint16_t idb)
{
  double db = idb / 64.0 - 512;
  return bse_db_to_factor (db);
}

}
