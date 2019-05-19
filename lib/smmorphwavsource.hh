// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_HH

#include "smmorphoperator.hh"

#include <string>

namespace SpectMorph
{

class MorphWavSource : public MorphOperator
{
  int         m_object_id = 0;
  int         m_INST      = 1;
  std::string m_lv2_filename;

public:
  MorphWavSource (MorphPlan *morph_plan);
  ~MorphWavSource();

  // inherited from MorphOperator
  const char        *type();
  int                insert_order();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  OutputType         output_type();

  void        set_object_id (int id);
  int         object_id();

  void        set_INST (int inst);
  int         INST();

  void        set_lv2_filename (const std::string& filename);
  std::string lv2_filename();
};

}

#endif
