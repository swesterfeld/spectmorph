// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_COMBOBOX_OPERATOR_HH
#define SPECTMORPH_COMBOBOX_OPERATOR_HH

#include "smmorphoperatorview.hh"
#include "smmorphlinear.hh"

#include <QComboBox>

namespace SpectMorph
{

struct OperatorFilter
{
  virtual bool filter (MorphOperator *op) = 0;
};

class ComboBoxOperator : public QWidget
{
  Q_OBJECT

protected:
  MorphPlan      *morph_plan;
  OperatorFilter *op_filter;
  MorphOperator  *op;
  std::string     str_choice;

  QComboBox      *combo_box;
  bool            block_changed;
  bool            none_ok;

  std::vector<std::string> str_choices;

protected slots:
  void on_combobox_changed();
  void on_operators_changed();

signals:
  void active_changed();

public:
  ComboBoxOperator (MorphPlan *plan, OperatorFilter *op_filter);

  void set_active (MorphOperator *new_op);
  MorphOperator *active();

  void add_str_choice (const std::string& str);

  std::string    active_str_choice();
  void           set_active_str_choice (const std::string& str);

  void           set_none_ok (bool none_ok);
};

// accepts all operators that have the required output type; rejects self
struct TypeOperatorFilter : public OperatorFilter
{
  MorphOperator *my_op;
  MorphOperator::OutputType type;

  TypeOperatorFilter (MorphOperator *my_op, MorphOperator::OutputType type) :
    my_op (my_op),
    type (type)
  {
    //
  }
  bool filter (MorphOperator *op)
  {
    return ((op != my_op) && op->output_type() == type);
  }
};

}

#endif
