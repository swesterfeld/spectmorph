// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_USER_INSTRUMENT_INDEX_HH
#define SPECTMORPH_USER_INSTRUMENT_INDEX_HH

#include "sminstrument.hh"

namespace SpectMorph
{

class UserInstrumentIndex
{
private:
  std::string user_instruments_dir;

public:
  UserInstrumentIndex()
  {
    user_instruments_dir = sm_get_documents_dir (DOCUMENTS_DIR_INSTRUMENTS);
  }
  void
  create_instrument_dir (const std::string& bank)
  {
    /* if bank directory doesn't exist, create it */
    auto dir = user_instruments_dir + "/" + bank;
    g_mkdir_with_parents (dir.c_str(), 0775);
  }
  std::string
  filename (const std::string& bank, int number)
  {
    return string_printf ("%s/%s/%d.sminst", user_instruments_dir.c_str(), bank.c_str(), number);
  }
  std::string
  label (const std::string& bank, int number)
  {
    Instrument inst;

    Error error = inst.load (filename (bank, number), Instrument::LoadOptions::NAME_ONLY);
    if (!error)
      return string_printf ("%03d %s", number, inst.name().c_str());
    else
      return string_printf ("%03d ---", number);
  }
};

}

#endif
