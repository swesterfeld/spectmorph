// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphgridview.hh"
#include "smmorphplan.hh"
#include "smcomboboxoperator.hh"
#include "smutils.hh"
#include "smoperatorlayout.hh"

using namespace SpectMorph;

using std::string;
using std::vector;
using std::set;

MorphGridView::MorphGridView (Widget *parent, MorphGrid *morph_grid, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_grid, morph_plan_window),
  morph_grid (morph_grid)
{
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

  // X / Y MORPHING
  pv_x_morphing = add_property_view (MorphGrid::P_X_MORPHING, op_layout);
  pv_y_morphing = add_property_view (MorphGrid::P_Y_MORPHING, op_layout);

  pv_x_morphing->set_show_control_status (false);
  pv_y_morphing->set_show_control_status (false);

  connect (pv_x_morphing->property()->signal_value_changed, [this]() { signal_grid_params_changed(); });
  connect (pv_y_morphing->property()->signal_value_changed, [this]() { signal_grid_params_changed(); });

  // Grid
  grid_widget = new MorphGridWidget (body_widget, morph_grid, this);
  op_layout.add_fixed (30, 30, grid_widget);

  connect (morph_plan_window->synth_interface()->signal_notify_event, grid_widget, &MorphGridWidget::on_synth_notify_event);
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

  update_visible();
}

double
MorphGridView::view_height()
{
  return op_layout.height() + 5;
}

void
MorphGridView::update_visible()
{
  pv_x_morphing->set_visible (true);
  pv_y_morphing->set_visible (true);

  op_layout.activate();
  signal_size_changed();
}

void
MorphGridView::on_grid_params_changed()
{
  width_label->set_text (string_printf ("%d", morph_grid->width()));
  height_label->set_text (string_printf ("%d", morph_grid->height()));
}

void
MorphGridView::on_operator_changed()
{
  if (morph_grid->has_selection())
    {
      MorphGridNode node = morph_grid->input_node (morph_grid->selected_x(), morph_grid->selected_y());

      node.op.set (op_combobox->active());
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
          g_assert (!node.op);
          string label = morph_grid->morph_plan()->index()->smset_to_label (node.smset);
          op_combobox->set_active_str_choice (label);
        }
      else
        {
          op_combobox->set_active (node.op.get());
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
