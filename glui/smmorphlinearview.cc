// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphlinearview.hh"
#include "smmorphplan.hh"
#include "smcomboboxoperator.hh"
#include "smoperatorlayout.hh"
#include "smcheckbox.hh"
#include "smutils.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphLinearView::MorphLinearView (Widget *parent, MorphLinear *morph_linear, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_linear, morph_plan_window),
  morph_linear (morph_linear)
{
  OperatorLayout op_layout;

  auto operator_filter = ComboBoxOperator::make_filter (morph_linear, MorphOperator::OUTPUT_AUDIO);

  // LEFT SOURCE
  left_combobox = new ComboBoxOperator (body_widget, morph_linear->morph_plan(), operator_filter);

  op_layout.add_row (3, new Label (body_widget, "Source A"), left_combobox);

  connect (left_combobox->signal_item_changed, this, &MorphLinearView::on_operator_changed);

  // RIGHT SOURCE
  right_combobox = new ComboBoxOperator (body_widget, morph_linear->morph_plan(), operator_filter);

  op_layout.add_row (3, new Label (body_widget, "Source B"), right_combobox);

  connect (right_combobox->signal_item_changed, this, &MorphLinearView::on_operator_changed);

  // CONTROL INPUT
  control_combobox = cv_control.create_combobox (body_widget,
    morph_linear,
    morph_linear->control_type(),
    morph_linear->control_op());
  connect (cv_control.signal_control_changed, this, &MorphLinearView::on_control_changed);

  op_layout.add_row (3, new Label (body_widget, "Control Input"), control_combobox);

  // MORPHING
  double morphing_slider_value = (morph_linear->morphing() + 1) / 2.0; /* restore value from operator */

  morphing_title = new Label (body_widget, "Morphing");
  morphing_slider = new Slider (body_widget, morphing_slider_value);
  morphing_label = new Label (body_widget, "0");
  op_layout.add_row (2, morphing_title, morphing_slider, morphing_label);

  connect (morphing_slider->signal_value_changed, this, &MorphLinearView::on_morphing_changed);

  on_morphing_changed (morphing_slider->value());
  update_slider();

  // FLAG: DB LINEAR
  CheckBox *db_linear_box = new CheckBox (body_widget, "dB Linear Morphing");
  db_linear_box->set_checked (morph_linear->db_linear());
  op_layout.add_row (2, db_linear_box);

  connect (db_linear_box->signal_toggled, [morph_linear] (bool new_value) {
    morph_linear->set_db_linear (new_value);
  });

  connect (morph_linear->morph_plan()->signal_index_changed, this, &MorphLinearView::on_index_changed);

  op_layout.activate();
  on_index_changed();     // add instruments to left/right combobox
}

double
MorphLinearView::view_height()
{
  return 18;
}

void
MorphLinearView::on_morphing_changed (double new_value)
{
  double dvalue = (new_value * 2) - 1;
  morphing_label->set_text (string_locale_printf ("%.2f", dvalue));
  morph_linear->set_morphing (dvalue);
}

void
MorphLinearView::on_control_changed()
{
  morph_linear->set_control_type_and_op (cv_control.control_type(), cv_control.op());

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
  const Index *index = morph_linear->morph_plan()->index();

  morph_linear->set_left_op (left_combobox->active());
  morph_linear->set_left_smset (index->label_to_smset (left_combobox->active_str_choice()));

  morph_linear->set_right_op (right_combobox->active());
  morph_linear->set_right_smset (index->label_to_smset (right_combobox->active_str_choice()));
}

void
MorphLinearView::on_db_linear_changed (bool new_value)
{
  morph_linear->set_db_linear (new_value);
}

void
MorphLinearView::on_index_changed()
{
  left_combobox->clear_str_choices();
  right_combobox->clear_str_choices();

  auto groups = morph_linear->morph_plan()->index()->groups();
  for (auto group : groups)
    {
      left_combobox->add_str_headline (group.group);
      right_combobox->add_str_headline (group.group);
      for (auto instrument : group.instruments)
        {
          left_combobox->add_str_choice (instrument.label);
          right_combobox->add_str_choice (instrument.label);
        }
    }
  left_combobox->set_op_headline ("Sources");
  right_combobox->set_op_headline ("Sources");

  if (morph_linear->left_op())
    left_combobox->set_active (morph_linear->left_op());

  if (morph_linear->left_smset() != "")
    {
      string label = morph_linear->morph_plan()->index()->smset_to_label (morph_linear->left_smset());
      left_combobox->set_active_str_choice (label);
    }
  if (morph_linear->right_op())
    right_combobox->set_active (morph_linear->right_op());

  if (morph_linear->right_smset() != "")
    {
      string label = morph_linear->morph_plan()->index()->smset_to_label (morph_linear->right_smset());
      right_combobox->set_active_str_choice (label);
    }
}
