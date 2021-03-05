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
  auto operator_filter = ComboBoxOperator::make_filter (morph_linear, MorphOperator::OUTPUT_AUDIO);

  // LEFT SOURCE
  left_combobox = new ComboBoxOperator (body_widget, morph_linear->morph_plan(), operator_filter);

  op_layout.add_row (3, new Label (body_widget, "Source A"), left_combobox);

  connect (left_combobox->signal_item_changed, this, &MorphLinearView::on_operator_changed);

  // RIGHT SOURCE
  right_combobox = new ComboBoxOperator (body_widget, morph_linear->morph_plan(), operator_filter);

  op_layout.add_row (3, new Label (body_widget, "Source B"), right_combobox);

  connect (right_combobox->signal_item_changed, this, &MorphLinearView::on_operator_changed);

  // MORPHING
  pv_morphing = add_property_view (MorphLinear::P_MORPHING, op_layout);

  // FLAG: DB LINEAR
  CheckBox *db_linear_box = new CheckBox (body_widget, "dB Linear Morphing");
  db_linear_box->set_checked (morph_linear->db_linear());
  op_layout.add_row (2, db_linear_box);

  connect (db_linear_box->signal_toggled, [morph_linear] (bool new_value) {
    morph_linear->set_db_linear (new_value);
  });

  connect (morph_linear->morph_plan()->signal_index_changed, this, &MorphLinearView::on_index_changed);

  update_visible();
  on_index_changed();     // add instruments to left/right combobox
}

double
MorphLinearView::view_height()
{
  return op_layout.height() + 5;
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

void
MorphLinearView::update_visible()
{
  pv_morphing->set_visible (true);

  op_layout.activate();
  signal_size_changed();
}
