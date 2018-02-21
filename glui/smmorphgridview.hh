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
  ComboBoxOperator   *op_combobox;

  MorphGridControlUI *x_ui;
  MorphGridControlUI *y_ui;

public:
  MorphGridView (Widget *parent, MorphGrid *op, MorphPlanWindow *morph_plan_window);

  double view_height() override;

  void on_plan_changed();
};

}

#endif
