// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_USER_INSTRUMENT_INDEX_HH
#define SPECTMORPH_USER_INSTRUMENT_INDEX_HH

#include "sminstrument.hh"

namespace SpectMorph
{

class UserInstrumentIndex
{
private:
  std::string user_bank_dir;

public:
  UserInstrumentIndex()
  {
    user_bank_dir = sm_get_documents_dir (DOCUMENTS_DIR_INSTRUMENTS) + "/User";
  }
  void
  create_instrument_dir()
  {
    /* if user bank directory doesn't exist, create it */
    g_mkdir_with_parents (user_bank_dir.c_str(), 0775);
  }
  std::string
  filename (int number)
  {
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
