// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_SOURCE_HH
#define SPECTMORPH_MORPH_SOURCE_HH

#include "smmorphoperator.hh"
#include "smwavset.hh"

#include <string>

namespace SpectMorph
{

class MorphSource : public MorphOperator
{
public:
  struct Config : public MorphOperatorConfig
  {
    WavSet *wav_set = nullptr;
  };
  Config      m_config;
protected:
  std::string m_smset;
public:
  MorphSource (MorphPlan *morph_plan);
  ~MorphSource();

  // inherited from MorphOperator
  const char        *type() override;
  int                insert_order() override;
  bool               save (OutFile& out_file) override;
  bool               load (InFile&  in_file) override;
  OutputType         output_type() override;
  MorphOperatorConfig *clone_config() override;

  void        set_smset (const std::string& smset);
  std::string smset();
};

}

#endif
