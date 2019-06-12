// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_USER_INSTRUMENT_INDEX_HH
#define SPECTMORPH_USER_INSTRUMENT_INDEX_HH

#include "sminstrument.hh"

namespace SpectMorph
{

class UserInstrumentIndex
{
public:
  std::string
  filename (int number)
  {
    std::string user_bank_dir = sm_get_user_dir (USER_DIR_DATA) + "/user"; // FIXME: test only
    g_mkdir_with_parents (user_bank_dir.c_str(), 0775);
    return string_printf ("%s/%d.sminst", user_bank_dir.c_str(), number);
  }
  std::string
  label (int number)
  {
    Instrument inst;

    Error error = inst.load (filename (number), Instrument::LoadOptions::NAME_ONLY);
    if (!error)
      return string_printf ("%03d %s", number, inst.name().c_str());
    else
      return string_printf ("%03d ---", number);
  }
};

}

#endif
