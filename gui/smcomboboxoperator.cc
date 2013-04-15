// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smcomboboxoperator.hh"

#include <assert.h>

#include <QComboBox>

using namespace SpectMorph;

using std::string;
using std::vector;

#define OPERATOR_TEXT_NONE "<none>"

ComboBoxOperator::ComboBoxOperator (MorphPlan *morph_plan, OperatorFilter *operator_filter) :
  morph_plan (morph_plan),
  op_filter (operator_filter),
  op (NULL)
{
  combo_box = new QComboBox();

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget (combo_box);
  layout->setContentsMargins (0, 0, 0, 0);
  setLayout (layout);

  none_ok = true;

  on_operators_changed();

  connect (combo_box, SIGNAL (currentIndexChanged (int)), this, SLOT (on_combobox_changed()));
  connect (morph_plan, SIGNAL (plan_changed()), this, SLOT (on_operators_changed()));
}

void
ComboBoxOperator::on_operators_changed()
{
  block_changed = true;

  combo_box->clear();

  if (none_ok)
    combo_box->addItem (OPERATOR_TEXT_NONE);

  for (vector<string>::const_iterator si = str_choices.begin(); si != str_choices.end(); si++)
    combo_box->addItem (si->c_str());

  const vector<MorphOperator *>& ops = morph_plan->operators();
  for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
    {
      MorphOperator *morph_op = *oi;
      if (op_filter->filter (morph_op))
        {
          combo_box->addItem (morph_op->name().c_str());
        }
    }

  // update selected item
  string active_name = OPERATOR_TEXT_NONE;
  if (op)
    active_name = op->name();
  if (str_choice != "")
    active_name = str_choice;

  int index = combo_box->findText (active_name.c_str());
  if (index >= 0)
    combo_box->setCurrentIndex (index);

  block_changed = false;
}

void
ComboBoxOperator::on_combobox_changed()
{
  if (block_changed)
    return;

  string active_text = combo_box->currentText().toLatin1().data();

  op         = NULL;
  str_choice = "";

  const vector<MorphOperator *>& ops = morph_plan->operators();
  for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
    {
      MorphOperator *morph_op = *oi;
      if (morph_op->name() == active_text)
        {
          op         = morph_op;
          str_choice = "";
        }
    }
  for (vector<string>::const_iterator si = str_choices.begin(); si != str_choices.end(); si++)
    {
      if (*si == active_text)
        {
          op         = NULL;
          str_choice = *si;
        }
    }
  Q_EMIT active_changed();
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
  for (vector<string>::const_iterator si = str_choices.begin(); si != str_choices.end(); si++)
    {
      if (new_str == *si)
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
