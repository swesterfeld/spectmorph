// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphlinearview.hh"
#include "smmorphplan.hh"
#include "smcomboboxoperator.hh"
#include "smutils.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

#define CONTROL_TEXT_GUI "Gui Slider"
#define CONTROL_TEXT_1   "Control Signal #1"
#define CONTROL_TEXT_2   "Control Signal #2"

MorphLinearView::MorphLinearView (Widget *parent, MorphLinear *morph_linear, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_linear, morph_plan_window),
  morph_linear (morph_linear)
{
  FixedGrid grid;

  int yoffset = 4;

  auto operator_filter = ComboBoxOperator::make_filter (morph_linear, MorphOperator::OUTPUT_AUDIO);
  auto control_operator_filter = ComboBoxOperator::make_filter (morph_linear, MorphOperator::OUTPUT_CONTROL);

  // LEFT SOURCE
  left_combobox = new ComboBoxOperator (this, morph_linear->morph_plan(), operator_filter);
  left_combobox->set_active (morph_linear->left_op());

  grid.add_widget (new Label (this, "Left Source"), 2, yoffset, 9, 3);
  grid.add_widget (left_combobox, 11, yoffset, 30, 3);

  connect (left_combobox->signal_item_changed, this, &MorphLinearView::on_operator_changed);

  yoffset += 3;

  // RIGHT SOURCE
  right_combobox = new ComboBoxOperator (this, morph_linear->morph_plan(), operator_filter);
  right_combobox->set_active (morph_linear->right_op());

  grid.add_widget (new Label (this, "Right Source"), 2, yoffset, 9, 3);
  grid.add_widget (right_combobox, 11, yoffset, 30, 3);

  connect (right_combobox->signal_item_changed, this, &MorphLinearView::on_operator_changed);

  // CONTROL INPUT

  yoffset += 3;

  control_combobox = new ComboBoxOperator (this, morph_linear->morph_plan(), control_operator_filter);
  control_combobox->add_str_choice (CONTROL_TEXT_GUI);
  control_combobox->add_str_choice (CONTROL_TEXT_1);
  control_combobox->add_str_choice (CONTROL_TEXT_2);
  control_combobox->set_none_ok (false);

  grid.add_widget (new Label (this, "Control Input"), 2, yoffset, 9, 3);
  grid.add_widget (control_combobox, 11, yoffset, 30, 3);

  connect (control_combobox->signal_item_changed, this, &MorphLinearView::on_control_changed);

  if (morph_linear->control_type() == MorphLinear::CONTROL_GUI) /* restore value */
    control_combobox->set_active_str_choice (CONTROL_TEXT_GUI);
  else if (morph_linear->control_type() == MorphLinear::CONTROL_SIGNAL_1)
    control_combobox->set_active_str_choice (CONTROL_TEXT_1);
  else if (morph_linear->control_type() == MorphLinear::CONTROL_SIGNAL_2)
    control_combobox->set_active_str_choice (CONTROL_TEXT_2);
  else if (morph_linear->control_type() == MorphLinear::CONTROL_OP)
    control_combobox->set_active (morph_linear->control_op());
  else
    {
      assert (false);
    }

  yoffset += 3;

  // MORPHING
  double morphing_slider_value = (morph_linear->morphing() + 1) / 2.0; /* restore value from operator */

  morphing_title = new Label (this, "Morphing");
  morphing_slider = new Slider (this, morphing_slider_value);
  morphing_label = new Label (this, "0");
  grid.add_widget (morphing_title, 2, yoffset, 9, 2);
  grid.add_widget (morphing_slider,  11, yoffset, 25, 2);
  grid.add_widget (morphing_label, 37, yoffset, 5, 2);

  connect (morphing_slider->signal_value_changed, this, &MorphLinearView::on_morphing_changed);

  on_morphing_changed (morphing_slider->value);
  update_slider();

#if 0
  // FLAG: DB LINEAR
  QCheckBox *db_linear_box = new QCheckBox ("dB Linear Morphing");
  db_linear_box->setChecked (morph_linear->db_linear());
  grid_layout->addWidget (db_linear_box, 4, 0, 1, 2);
  connect (db_linear_box, SIGNAL (toggled (bool)), this, SLOT (on_db_linear_changed (bool)));
#endif
}

double
MorphLinearView::view_height()
{
  return 16;
}

void
MorphLinearView::on_morphing_changed (double new_value)
{
  double dvalue = (new_value * 2) - 1;
  morphing_label->text = string_locale_printf ("%.2f", dvalue);
  morph_linear->set_morphing (dvalue);
}

void
MorphLinearView::on_control_changed()
{
  MorphOperator *op = control_combobox->active();
  if (op)
    {
      morph_linear->set_control_op (op);
      morph_linear->set_control_type (MorphLinear::CONTROL_OP);
    }
  else
    {
      string text = control_combobox->active_str_choice();

      if (text == CONTROL_TEXT_GUI)
        morph_linear->set_control_type (MorphLinear::CONTROL_GUI);
      else if (text == CONTROL_TEXT_1)
        morph_linear->set_control_type (MorphLinear::CONTROL_SIGNAL_1);
      else if (text == CONTROL_TEXT_2)
        morph_linear->set_control_type (MorphLinear::CONTROL_SIGNAL_2);
      else
        {
          assert (false);
        }
    }
  update_slider();
}

void
MorphLinearView::update_slider()
{
  bool enabled = (morph_linear->control_type() == MorphLinear::CONTROL_GUI);

  morphing_title->set_enabled (enabled);
  morphing_slider->set_enabled (enabled);
  morphing_label->set_enabled (enabled);
}

void
MorphLinearView::on_operator_changed()
{
  morph_linear->set_left_op (left_combobox->active());
  morph_linear->set_right_op (right_combobox->active());
}

void
MorphLinearView::on_db_linear_changed (bool new_value)
{
  morph_linear->set_db_linear (new_value);
}
