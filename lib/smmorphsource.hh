// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_SOURCE_HH
#define SPECTMORPH_MORPH_SOURCE_HH

#include "smmorphoperator.hh"

#include <string>

namespace SpectMorph
{

class MorphSource : public MorphOperator
{
  std::string m_smset;
public:
  MorphSource (MorphPlan *morph_plan);
  ~MorphSource();

  // inherited from MorphOperator
  const char        *type();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  OutputType         output_type();

  void        set_smset (const std::string& smset);
  std::string smset();
};

}

#endif
