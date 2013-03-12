// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_HH
#define SPECTMORPH_MORPH_OUTPUT_HH

#include "smmorphoperator.hh"

#include <string>

namespace SpectMorph
{

class MorphOutput : public MorphOperator
{
  Q_OBJECT

  std::vector<std::string>     load_channel_op_names;
  std::vector<MorphOperator *> channel_ops;

  bool                         m_sines;
  bool                         m_noise;

public:
  MorphOutput (MorphPlan *morph_plan);
  ~MorphOutput();

  // inherited from MorphOperator
  const char        *type();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  void               post_load (OpNameMap& op_name_map);
  OutputType         output_type();

  void           set_sines (bool es);
  bool           sines();

  void           set_noise (bool en);
  bool           noise();

  void           set_channel_op (int ch, MorphOperator *op);
  MorphOperator *channel_op (int ch);

public slots:
  void on_operator_removed (MorphOperator *op);
};

}

#endif
