// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_HH

#include "smmorphoperator.hh"

#include <string>

namespace SpectMorph
{

class MorphWavSource : public MorphOperator
{
  std::string m_instrument;
  int64       m_instrument_id = 0;
  void update_instrument_and_id (const std::string& new_instrument);

public:
  MorphWavSource (MorphPlan *morph_plan);
  ~MorphWavSource();

  // inherited from MorphOperator
  const char        *type();
  int                insert_order();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  OutputType         output_type();

  void        set_instrument (const std::string& instrument);
  std::string instrument();

  int64       instrument_id();
};

}

#endif
