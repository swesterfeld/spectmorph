// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridview.hh"
#include "smmorphplan.hh"
#include "smcomboboxoperator.hh"
#include "smutils.hh"

using namespace SpectMorph;

using std::string;
using std::vector;
using std::set;

#define CONTROL_TEXT_GUI "Gui Slider"
#define CONTROL_TEXT_1   "Control Signal #1"
#define CONTROL_TEXT_2   "Control Signal #2"

MorphGridControlUI::MorphGridControlUI (MorphGridView *parent, MorphGrid *morph_grid, Widget *body_widget, ControlXYType ctl_xy) :
  morph_grid (morph_grid),
  ctl_xy (ctl_xy)
{
  auto control_operator_filter = ComboBoxOperator::make_filter (morph_grid, MorphOperator::OUTPUT_CONTROL);
  combobox = new ComboBoxOperator (body_widget, morph_grid->morph_plan(), control_operator_filter);
  combobox->add_str_choice (CONTROL_TEXT_GUI);
  combobox->add_str_choice (CONTROL_TEXT_1);
  combobox->add_str_choice (CONTROL_TEXT_2);
  combobox->set_none_ok (false);

  /* restore initial combobox state */
  MorphGrid::ControlType control_type = (ctl_xy == CONTROL_X) ?
                                        morph_grid->x_control_type() :
                                        morph_grid->y_control_type();


  if (control_type == MorphGrid::CONTROL_GUI)
    combobox->set_active_str_choice (CONTROL_TEXT_GUI);
  else if (control_type == MorphGrid::CONTROL_SIGNAL_1)
    combobox->set_active_str_choice (CONTROL_TEXT_1);
  else if (control_type == MorphGrid::CONTROL_SIGNAL_2)
    combobox->set_active_str_choice (CONTROL_TEXT_2);
  else if (control_type == MorphGrid::CONTROL_OP)
    {
      if (ctl_xy == CONTROL_X)
        {
          combobox->set_active (morph_grid->x_control_op());
        }
      else
        {
          combobox->set_active (morph_grid->y_control_op());
        }
    }
  else
    {
      assert (false);
    }

  title = new Label (body_widget, ctl_xy == CONTROL_X ? "X Value" : "Y Value");
  slider = new Slider (body_widget, 0);
  label = new Label (body_widget, "");

  /* restore slider value from operator */
  if (ctl_xy == CONTROL_X)
    slider->value = (morph_grid->x_morphing() + 1) / 2;
  else
    slider->value = (morph_grid->y_morphing() + 1) / 2;

  connect (slider->signal_value_changed, this, &MorphGridControlUI::on_slider_changed);
  connect (combobox->signal_item_changed, this, &MorphGridControlUI::on_combobox_changed);

  // initial slider state
  on_combobox_changed();
}

void
MorphGridControlUI::on_slider_changed (double value)
{
  value = value * 2 - 1;

  if (ctl_xy == CONTROL_X)
    morph_grid->set_x_morphing (value);
  else
    morph_grid->set_y_morphing (value);
}

void
MorphGridControlUI::on_combobox_changed()
{
  bool control_gui = false;

  MorphOperator *op = combobox->active();
  if (op)
    {
      if (ctl_xy == CONTROL_X)
        {
          morph_grid->set_x_control_op (op);
          morph_grid->set_x_control_type (MorphGrid::CONTROL_OP);
        }
      else
        {
          morph_grid->set_y_control_op (op);
          morph_grid->set_y_control_type (MorphGrid::CONTROL_OP);
        }
    }
  else
    {
      string text = combobox->active_str_choice();
      MorphGrid::ControlType new_type;

      if (text == CONTROL_TEXT_GUI)
        {
          new_type = MorphGrid::CONTROL_GUI;

          control_gui = true;
        }
      else if (text == CONTROL_TEXT_1)
        new_type = MorphGrid::CONTROL_SIGNAL_1;
      else if (text == CONTROL_TEXT_2)
        new_type = MorphGrid::CONTROL_SIGNAL_2;
      else
        {
          assert (false);
        }
      if (ctl_xy == CONTROL_X)
        morph_grid->set_x_control_type (new_type);
      else
        morph_grid->set_y_control_type (new_type);
    }
  title->set_enabled (control_gui);
  label->set_enabled (control_gui);
  slider->set_enabled (control_gui);
}



MorphGridView::MorphGridView (Widget *parent, MorphGrid *morph_grid, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_grid, morph_plan_window),
  morph_grid (morph_grid)
{
  FixedGrid grid;

  int yoffset = 0;

  // Width
  Label *width_title   = new Label (body_widget, "Width");
  Slider *width_slider = new Slider (body_widget, 0);
  width_slider->set_int_range (1, 7);
  width_label = new Label (body_widget, "0");

  grid.add_widget (width_title, 0, yoffset, 9, 2);
  grid.add_widget (width_slider,  9, yoffset, 25, 2);
  grid.add_widget (width_label, 35, yoffset, 5, 2);

  width_slider->set_int_value (morph_grid->width());

  connect (width_slider->signal_int_value_changed, [=] (int width) { morph_grid->set_width (width); });

  yoffset += 2;

  // Height
  Label *height_title   = new Label (body_widget, "Height");
  Slider *height_slider = new Slider (body_widget, 0);
  height_slider->set_int_range (1, 7);
  height_label = new Label (body_widget, "0");

  grid.add_widget (height_title, 0, yoffset, 9, 2);
  grid.add_widget (height_slider,  9, yoffset, 25, 2);
  grid.add_widget (height_label, 35, yoffset, 5, 2);

  height_slider->set_int_value (morph_grid->height());

  connect (height_slider->signal_int_value_changed, [=] (int height) { morph_grid->set_height (height); });

  yoffset += 2;

  // X Control
  x_ui = new MorphGridControlUI (this, morph_grid, body_widget, MorphGridControlUI::CONTROL_X);
  grid.add_widget (new Label (body_widget, "X Control"), 0, yoffset, 9, 3);
  grid.add_widget (x_ui->combobox, 9, yoffset, 30, 3);

  yoffset += 3;

  // Y Control
  y_ui = new MorphGridControlUI (this, morph_grid, body_widget, MorphGridControlUI::CONTROL_Y);
  grid.add_widget (new Label (body_widget, "Y Control"), 0, yoffset, 9, 3);
  grid.add_widget (y_ui->combobox, 9, yoffset, 30, 3);

  yoffset += 3;

  // X Value
  grid.add_widget (x_ui->title, 0, yoffset, 9, 2);
  grid.add_widget (x_ui->slider, 9, yoffset, 25, 2);
  grid.add_widget (x_ui->label, 35, yoffset, 5, 2);

  yoffset += 2;

  // Y Value
  grid.add_widget (y_ui->title, 0, yoffset, 9, 2);
  grid.add_widget (y_ui->slider,  9, yoffset, 25, 2);
  grid.add_widget (y_ui->label, 35, yoffset, 5, 2);

  yoffset += 2;

  // Grid
  yoffset++;
  grid_widget = new MorphGridWidget (body_widget, morph_grid);
  grid.add_widget (grid_widget, 5, yoffset, 30, 30);

  connect (grid_widget->signal_selection_changed, this, &MorphGridView::on_selection_changed);

  yoffset += 31;

  // Source
  auto input_op_filter = ComboBoxOperator::make_filter (morph_grid, MorphOperator::OUTPUT_AUDIO);
  op_title = new Label (body_widget, "Source");
  op_combobox = new ComboBoxOperator (body_widget, morph_grid->morph_plan(), input_op_filter);
  grid.add_widget (op_title, 0, yoffset, 9, 3);
  grid.add_widget (op_combobox, 9, yoffset, 30, 3);

  connect (op_combobox->signal_item_changed, this, &MorphGridView::on_operator_changed);

  yoffset += 3;

  // Volume
  delta_db_title = new Label (body_widget, "Volume");
  delta_db_slider = new Slider (body_widget, 0.5);
  delta_db_label = new Label (body_widget, string_locale_printf ("%.1f dB", 0.0));

  grid.add_widget (delta_db_title, 0, yoffset, 9, 2);
  grid.add_widget (delta_db_slider, 9, yoffset, 25, 2);
  grid.add_widget (delta_db_label, 35, yoffset, 5, 2);

  connect (delta_db_slider->signal_value_changed, this, &MorphGridView::on_delta_db_changed);

  // Global
  connect (morph_grid->morph_plan()->signal_plan_changed, this, &MorphGridView::on_plan_changed);
  connect (morph_grid->morph_plan()->signal_index_changed, this, &MorphGridView::on_index_changed);

  on_index_changed();     // add instruments to op_combobox
  on_plan_changed();      // initial morphing slider/label setup
  on_selection_changed(); // initial selection
}

double
MorphGridView::view_height()
{
  return 56;
}

void
MorphGridView::on_plan_changed()
{
  width_label->text = string_printf ("%d", morph_grid->width());
  height_label->text = string_printf ("%d", morph_grid->height());
  x_ui->slider->value = (morph_grid->x_morphing() + 1) / 2;
  y_ui->slider->value = (morph_grid->y_morphing() + 1) / 2;
  x_ui->label->text = string_locale_printf ("%.2f", morph_grid->x_morphing());
  y_ui->label->text = string_locale_printf ("%.2f", morph_grid->y_morphing());
}

void
MorphGridView::on_operator_changed()
{
  if (morph_grid->has_selection())
    {
      MorphGridNode node = morph_grid->input_node (morph_grid->selected_x(), morph_grid->selected_y());

      node.op = op_combobox->active();
      node.smset = op_combobox->active_str_choice();

      morph_grid->set_input_node (morph_grid->selected_x(), morph_grid->selected_y(), node);
    }
}

void
MorphGridView::on_delta_db_changed (double new_value)
{
  const double db = (new_value * 2 - 1) * 48;

  delta_db_label->text = string_locale_printf ("%.1f dB", db);

  if (morph_grid->has_selection())
    {
      MorphGridNode node = morph_grid->input_node (morph_grid->selected_x(), morph_grid->selected_y());

      node.delta_db = db;

      morph_grid->set_input_node (morph_grid->selected_x(), morph_grid->selected_y(), node);
    }
}

void
MorphGridView::on_selection_changed()
{
  op_title->set_enabled (morph_grid->has_selection());
  op_combobox->set_enabled (morph_grid->has_selection());
  delta_db_title->set_enabled (morph_grid->has_selection());
  delta_db_slider->set_enabled (morph_grid->has_selection());
  delta_db_label->set_enabled (morph_grid->has_selection());

  if (morph_grid->has_selection())
    {
      MorphGridNode node = morph_grid->input_node (morph_grid->selected_x(), morph_grid->selected_y());

      if (node.smset != "")
        {
          g_assert (node.op == NULL);
          op_combobox->set_active_str_choice (node.smset);
        }
      else
        {
          op_combobox->set_active (node.op);
        }
      delta_db_slider->value = (node.delta_db / 48 + 1) / 2;
    }
}

void
MorphGridView::on_index_changed()
{
  op_combobox->clear_str_choices();

  vector<string> smsets = morph_grid->morph_plan()->index()->smsets();
  for (vector<string>::iterator si = smsets.begin(); si != smsets.end(); si++)
    op_combobox->add_str_choice (*si);

  set<string> smset_set (smsets.begin(), smsets.end());
  for (int x = 0; x < morph_grid->width(); x++)
    {
      for (int y = 0; y < morph_grid->height(); y++)
        {
          MorphGridNode node = morph_grid->input_node (x, y);

          if (node.smset != "" && !smset_set.count (node.smset))
            {
              // instrument not present in new index, remove
              // (probably should not be done in gui code)
              node.smset = "";
              morph_grid->set_input_node (x, y, node);
            }
        }
    }
}
