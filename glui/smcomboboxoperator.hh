// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_COMBOBOX_OPERATOR_HH
#define SPECTMORPH_COMBOBOX_OPERATOR_HH

#include "smcombobox.hh"
#include "smmorphoperator.hh"

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
  std::string     op_headline;

  ComboBox       *combobox;
  bool            none_ok;

  struct StrItem
  {
    bool        headline;
    std::string text;
  };
  std::vector<StrItem> str_items;

public:
  ComboBoxOperator (Widget *parent, MorphPlan *plan, const OperatorFilter& op_filter);

  void set_active (MorphOperator *new_op);
  MorphOperator *active();

  void           add_str_headline (const std::string& str);
  void           add_str_choice (const std::string& str);
  void           clear_str_choices();

  std::string    active_str_choice();
  void           set_active_str_choice (const std::string& str);

  void           set_none_ok (bool none_ok);
  void           set_op_headline (const std::string& headline);

  static OperatorFilter
  make_filter (MorphOperator *my_op, MorphOperator::OutputType type)
  {
    return [my_op, type] (MorphOperator *op) -> bool { return ((op != my_op) && op->output_type() == type); };
  }

  /* signals */
  Signal<> signal_item_changed;

  /* slots */
  void on_operators_changed();
  void on_combobox_changed();
  void on_update_combobox_geometry();
};

}

#endif

