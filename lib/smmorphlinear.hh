// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_LINEAR_HH
#define SPECTMORPH_MORPH_LINEAR_HH

#include "smmorphoperator.hh"

#include <string>

namespace SpectMorph
{

class MorphLinear : public MorphOperator
{
  Q_OBJECT
public:
  enum ControlType {
    CONTROL_GUI      = 1,
    CONTROL_SIGNAL_1 = 2,
    CONTROL_SIGNAL_2 = 3,
    CONTROL_OP       = 4
  };
protected:
  std::string    load_left, load_right, load_control;

  MorphOperator *m_left_op;
  MorphOperator *m_right_op;
  MorphOperator *m_control_op;
  double         m_morphing;
  ControlType    m_control_type;
  bool           m_db_linear;
  bool           m_use_lpc;

public:
  MorphLinear (MorphPlan *morph_plan);
  ~MorphLinear();

  // inherited from MorphOperator
  const char        *type();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  void               post_load();
  OutputType         output_type();

  MorphOperator *left_op();
  void set_left_op (MorphOperator *op);

  MorphOperator *right_op();
  void set_right_op (MorphOperator *op);

  MorphOperator *control_op();
  void set_control_op (MorphOperator *op);

  double morphing();
  void set_morphing (double new_morphing);

  ControlType control_type();
  void set_control_type (ControlType new_control_type);

  bool db_linear();
  void set_db_linear (bool new_db_linear);

  bool use_lpc();
  void set_use_lpc (bool new_use_lpc);

public slots:
  void on_operator_removed (MorphOperator *op);
};

}

#endif
