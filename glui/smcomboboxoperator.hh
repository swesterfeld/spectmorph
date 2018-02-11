// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_COMBOBOX_OPERATOR_HH
#define SPECTMORPH_COMBOBOX_OPERATOR_HH

#include "smmorphoperatorview.hh"
#include "smmorphlinear.hh"

#include "smcombobox.hh"

namespace SpectMorph
{

typedef std::function<bool(MorphOperator*)> OperatorFilter;

class ComboBoxOperator : public Widget
{
protected:
  MorphPlan      *morph_plan;
  OperatorFilter  op_filter;
  MorphOperator  *op;
  std::string     str_choice;

  ComboBox       *combobox;
  bool            none_ok;

  std::vector<std::string> str_choices;

public:
  ComboBoxOperator (Widget *parent, MorphPlan *plan, const OperatorFilter& op_filter);

  void set_active (MorphOperator *new_op);
  MorphOperator *active();

  /* signals */
  Signal<> signal_item_changed;

  /* slots */
  void on_operators_changed();
  void on_combobox_changed();
};

}

#endif

