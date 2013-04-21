// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmath.hh"
#include <bse/bsemathsignal.hh>

namespace SpectMorph {

int16_t
sm_factor2idb (double factor)
{
  double db = bse_db_from_factor (factor, -500);

  return db < 0 ? int (db * 64 - 0.5) : int (db * 64 + 0.5);
}

double
sm_idb2factor (int16_t idb)
{
  double db = idb / 64.0;
  return bse_db_to_factor (db);
}

}
