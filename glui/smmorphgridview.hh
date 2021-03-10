// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_VIEW_HH
#define SPECTMORPH_MORPH_GRID_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphgrid.hh"
#include "smcomboboxoperator.hh"
#include "smmorphgridwidget.hh"
#include "smcontrolview.hh"
#include "smoperatorlayout.hh"

namespace SpectMorph
{

class MorphGridView;

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

  PropertyView       *pv_x_morphing;
  PropertyView       *pv_y_morphing;
  OperatorLayout      op_layout;

  void update_db_label (double db);

public:
  MorphGridView (Widget *parent, MorphGrid *op, MorphPlanWindow *morph_plan_window);

  double view_height() override;
  void update_visible() override;

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
