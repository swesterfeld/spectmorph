// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridview.hh"
#include "smmorphplan.hh"
#include "smcomboboxoperator.hh"
#include "smutils.hh"
#include "smoperatorlayout.hh"

using namespace SpectMorph;

using std::string;
using std::vector;
using std::set;

#define CONTROL_TEXT_GUI "Gui Slider"
#define CONTROL_TEXT_1   "Control Signal #1"
#define CONTROL_TEXT_2   "Control Signal #2"

MorphGridControlUI::MorphGridControlUI (MorphGridView *morph_grid_view, MorphGrid *morph_grid, Widget *body_widget, ControlXYType ctl_xy) :
  morph_grid (morph_grid),
  morph_grid_view (morph_grid_view),
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
      g_assert_not_reached();
    }

  title = new Label (body_widget, ctl_xy == CONTROL_X ? "X Value" : "Y Value");
  slider = new Slider (body_widget, 0);
  label = new Label (body_widget, "");

  /* restore slider value from operator */
  if (ctl_xy == CONTROL_X)
    slider->set_value ((morph_grid->x_morphing() + 1) / 2);
  else
    slider->set_value ((morph_grid->y_morphing() + 1) / 2);

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

  morph_grid_view->signal_grid_params_changed();
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
          g_assert_not_reached();
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
  OperatorLayout op_layout;

  // Width
  Label *width_title   = new Label (body_widget, "Width");
  Slider *width_slider = new Slider (body_widget, 0);
  width_slider->set_int_range (1, 7);
  width_label = new Label (body_widget, "0");

  op_layout.add_row (2, width_title, width_slider, width_label);

  width_slider->set_int_value (morph_grid->width());

  connect (width_slider->signal_int_value_changed, [=] (int width) {
    morph_grid->set_width (width);
    signal_grid_params_changed();
  });

  // Height
  Label *height_title   = new Label (body_widget, "Height");
  Slider *height_slider = new Slider (body_widget, 0);
  height_slider->set_int_range (1, 7);
  height_label = new Label (body_widget, "0");

  op_layout.add_row (2, height_title, height_slider, height_label);

  height_slider->set_int_value (morph_grid->height());

  connect (height_slider->signal_int_value_changed, [=] (int height) {
    morph_grid->set_height (height);
    signal_grid_params_changed();
  });

  // X Control
  x_ui = new MorphGridControlUI (this, morph_grid, body_widget, MorphGridControlUI::CONTROL_X);
  op_layout.add_row (3, new Label (body_widget, "X Control"), x_ui->combobox);

  // Y Control
  y_ui = new MorphGridControlUI (this, morph_grid, body_widget, MorphGridControlUI::CONTROL_Y);
  op_layout.add_row (3, new Label (body_widget, "Y Control"), y_ui->combobox);

  // X Value
  op_layout.add_row (2, x_ui->title, x_ui->slider, x_ui->label);

  // Y Value
  op_layout.add_row (2, y_ui->title, y_ui->slider, y_ui->label);

  // Grid
  grid_widget = new MorphGridWidget (body_widget, morph_grid, this);
  op_layout.add_fixed (30, 30, grid_widget);

  connect (grid_widget->signal_selection_changed, this, &MorphGridView::on_selection_changed);

  // Source
  auto input_op_filter = ComboBoxOperator::make_filter (morph_grid, MorphOperator::OUTPUT_AUDIO);
  op_title = new Label (body_widget, "Source");
  op_combobox = new ComboBoxOperator (body_widget, morph_grid->morph_plan(), input_op_filter);
  op_layout.add_row (3, op_title, op_combobox);

  connect (op_combobox->signal_item_changed, this, &MorphGridView::on_operator_changed);

  // Volume
  delta_db_title = new Label (body_widget, "Volume");
  delta_db_slider = new Slider (body_widget, 0.5);
  delta_db_label = new Label (body_widget, string_locale_printf ("%.1f dB", 0.0));

  op_layout.add_row (2, delta_db_title, delta_db_slider, delta_db_label);

  connect (delta_db_slider->signal_value_changed, this, &MorphGridView::on_delta_db_changed);

  // state updates to perform if some grid parameter changes
  connect (signal_grid_params_changed, this, &MorphGridView::on_grid_params_changed);
  connect (grid_widget->signal_grid_params_changed, this, &MorphGridView::on_grid_params_changed);

  // Global
  connect (morph_grid->morph_plan()->signal_index_changed, this, &MorphGridView::on_index_changed);

  on_index_changed();       // add instruments to op_combobox
  on_grid_params_changed(); // initial morphing slider/label setup
  on_selection_changed();   // initial selection

  op_layout.activate();
}

double
MorphGridView::view_height()
{
  return 56;
}

void
MorphGridView::on_grid_params_changed()
{
  width_label->set_text (string_printf ("%d", morph_grid->width()));
  height_label->set_text (string_printf ("%d", morph_grid->height()));
  x_ui->slider->set_value ((morph_grid->x_morphing() + 1) / 2);
  y_ui->slider->set_value ((morph_grid->y_morphing() + 1) / 2);
  x_ui->label->set_text (string_locale_printf ("%.2f", morph_grid->x_morphing()));
  y_ui->label->set_text (string_locale_printf ("%.2f", morph_grid->y_morphing()));
}

void
MorphGridView::on_operator_changed()
{
  if (morph_grid->has_selection())
    {
      MorphGridNode node = morph_grid->input_node (morph_grid->selected_x(), morph_grid->selected_y());

      node.op = op_combobox->active();
      node.smset = morph_grid->morph_plan()->index()->label_to_smset (op_combobox->active_str_choice());

      morph_grid->set_input_node (morph_grid->selected_x(), morph_grid->selected_y(), node);

      signal_grid_params_changed();
    }
}

void
MorphGridView::update_db_label (double db)
{
  delta_db_label->set_text (string_locale_printf ("%.1f dB", db));
}

void
MorphGridView::on_delta_db_changed (double new_value)
{
  const double db = (new_value * 2 - 1) * 48;

  update_db_label (db);

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
          string label = morph_grid->morph_plan()->index()->smset_to_label (node.smset);
          op_combobox->set_active_str_choice (label);
        }
      else
        {
          op_combobox->set_active (node.op);
        }
      delta_db_slider->set_value ((node.delta_db / 48 + 1) / 2);
      update_db_label (node.delta_db);
    }
}

void
MorphGridView::on_index_changed()
{
  op_combobox->clear_str_choices();

  set<string> smset_set;
  auto groups = morph_grid->morph_plan()->index()->groups();
  for (auto group : groups)
    {
      op_combobox->add_str_headline (group.group);
      for (auto instrument : group.instruments)
        {
          op_combobox->add_str_choice (instrument.label);
          smset_set.insert (instrument.smset);
        }
    }
  op_combobox->set_op_headline ("Sources");

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
