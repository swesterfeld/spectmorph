// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LINEAR_HH
#define SPECTMORPH_MORPH_LINEAR_HH

#include "smmorphoperator.hh"

#include <string>

namespace SpectMorph
{

class MorphLinear : public MorphOperator
{
protected:
  std::string    load_left, load_right, load_control;

  MorphOperator *m_left_op;
  std::string    m_left_smset;
  MorphOperator *m_right_op;
  std::string    m_right_smset;
  MorphOperator *m_control_op;
  double         m_morphing;
  ControlType    m_control_type;
  bool           m_db_linear;

public:
  MorphLinear (MorphPlan *morph_plan);
  ~MorphLinear();

  // inherited from MorphOperator
  const char        *type() override;
  int                insert_order() override;
  bool               save (OutFile& out_file) override;
  bool               load (InFile&  in_file) override;
  void               post_load (OpNameMap& op_name_map) override;
  OutputType         output_type() override;

  std::vector<MorphOperator *> dependencies() override;

  MorphOperator *left_op();
  void set_left_op (MorphOperator *op);

  std::string left_smset();
  void set_left_smset (const std::string& smset);

  MorphOperator *right_op();
  void set_right_op (MorphOperator *op);

  std::string right_smset();
  void set_right_smset (const std::string& smset);

  MorphOperator *control_op();
  void set_control_op (MorphOperator *op);

  double morphing();
  void set_morphing (double new_morphing);

  ControlType control_type();
  void set_control_type (ControlType new_control_type);

  bool db_linear();
  void set_db_linear (bool new_db_linear);

/* slots: */
  void on_operator_removed (MorphOperator *op);
};

}

#endif
