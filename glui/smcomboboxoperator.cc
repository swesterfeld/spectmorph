// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smcomboboxoperator.hh"

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

#define OPERATOR_TEXT_NONE "<none>"

ComboBoxOperator::ComboBoxOperator (Widget *parent, MorphPlan *morph_plan, const OperatorFilter& operator_filter) :
  Widget (parent),
  morph_plan (morph_plan),
  op_filter (operator_filter),
  op (NULL)
{
  combobox = new ComboBox (this);
  /* FIXME: need elegant way to propagate size to child */
  combobox->x = 0;
  combobox->y = 0;
  combobox->width = 30*8;
  combobox->height = 24;

  none_ok = true;

  on_operators_changed();

  connect (combobox->signal_item_changed, this, &ComboBoxOperator::on_combobox_changed);
  connect (morph_plan->signal_plan_changed, this, &ComboBoxOperator::on_operators_changed);
}

void
ComboBoxOperator::on_operators_changed()
{
  combobox->clear();

  if (none_ok)
    combobox->add_item (OPERATOR_TEXT_NONE);

  for (auto str : str_choices)
    combobox->add_item (str);

  for (MorphOperator *morph_op : morph_plan->operators())
    {
      if (op_filter (morph_op))
        {
          combobox->add_item (morph_op->name());
        }
    }

  // update selected item
  string active_name = OPERATOR_TEXT_NONE;
  if (op)
    active_name = op->name();
  if (str_choice != "")
    active_name = str_choice;

  combobox->set_text (active_name);
}

void
ComboBoxOperator::on_combobox_changed()
{
  string active_text = combobox->text();

  op         = NULL;
  str_choice = "";

  for (MorphOperator *morph_op : morph_plan->operators())
    {
      if (morph_op->name() == active_text)
        {
          op         = morph_op;
          str_choice = "";
        }
    }
  for (auto str : str_choices)
    {
      if (str == active_text)
        {
          op         = NULL;
          str_choice = str;
        }
    }
  signal_item_changed();
}

void
ComboBoxOperator::set_active (MorphOperator *new_op)
{
  op         = new_op;
  str_choice = "";

  on_operators_changed();
}

MorphOperator *
ComboBoxOperator::active()
{
  return op;
}

void
ComboBoxOperator::add_str_choice (const string& str)
{
  str_choices.push_back (str);

  on_operators_changed();
}

void
ComboBoxOperator::clear_str_choices()
{
  str_choices.clear();
  str_choice = "";

  on_operators_changed();
}

string
ComboBoxOperator::active_str_choice()
{
  return str_choice;
}

void
ComboBoxOperator::set_active_str_choice (const string& new_str)
{
  for (auto str : str_choices)
    {
      if (new_str == str)
        {
          op         = NULL;
          str_choice = new_str;

          on_operators_changed();

          return;
        }
    }
  printf ("ComboBoxOperator::set_active_str_choice (%s) failed\n", new_str.c_str());
  g_assert_not_reached();
}

void
ComboBoxOperator::set_none_ok (bool new_none_ok)
{
  none_ok = new_none_ok;

  on_operators_changed();
}
