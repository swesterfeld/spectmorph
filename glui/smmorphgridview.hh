// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_VIEW_HH
#define SPECTMORPH_MORPH_GRID_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphgrid.hh"
#include "smcomboboxoperator.hh"
#include "smmorphgridwidget.hh"

namespace SpectMorph
{

class MorphGridView;
class MorphGridControlUI : public SignalReceiver
{
  MorphGrid          *morph_grid;
  MorphGridView      *morph_grid_view;

public:
  enum ControlXYType { CONTROL_X, CONTROL_Y } ctl_xy;
  MorphGridControlUI (MorphGridView *parent, MorphGrid *morph_grid, Widget *body_widget, ControlXYType ctl_xy);

  ComboBoxOperator *combobox;
  Label            *title;
  Slider           *slider;
  Label            *label;
  double            value;

/* slots: */
  void on_slider_changed (double value);
  void on_combobox_changed();
};

class MorphGridView : public MorphOperatorView
{
protected:
  MorphGrid          *morph_grid;
  MorphGridWidget    *grid_widget;
  Label              *width_label;
  Label              *height_label;
  Label              *op_title;
  ComboBoxOperator   *op_combobox;
  Label              *delta_db_title;
  Label              *delta_db_label;
  Slider             *delta_db_slider;

  MorphGridControlUI *x_ui;
  MorphGridControlUI *y_ui;

public:
  MorphGridView (Widget *parent, MorphGrid *op, MorphPlanWindow *morph_plan_window);

  double view_height() override;

/* signals: */
  Signal<> signal_grid_params_changed;

/* slots: */
  void on_grid_params_changed();
  void on_delta_db_changed (double new_value);
  void on_selection_changed();
  void on_index_changed();
  void on_operator_changed();
};

}

#endif
