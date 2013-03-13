// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_VIEW_HH
#define SPECTMORPH_MORPH_GRID_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphgrid.hh"
#include "smcomboboxoperator.hh"
#include "smmorphgridwidget.hh"

#include <QSpinBox>
#include <QStackedWidget>

namespace SpectMorph
{

class MorphGridView;
class MorphGridControlUI : public QObject
{
  Q_OBJECT

  TypeOperatorFilter  control_op_filter;
  MorphGrid          *morph_grid;

public:
  enum ControlXYType { CONTROL_X, CONTROL_Y } ctl_xy;
  MorphGridControlUI (MorphGridView *parent, MorphGrid *morph_grid, ControlXYType ctl_xy);

  ComboBoxOperator *combobox;
  QSlider          *slider;
  QLabel           *label;
  QStackedWidget   *stack;
  double            value;

public slots:
  void on_slider_changed();
  void on_combobox_changed();
};

class MorphGridView : public MorphOperatorView
{
  Q_OBJECT

  QSpinBox           *width_spinbox;
  QSpinBox           *height_spinbox;

  MorphGridControlUI *x_ui;
  MorphGridControlUI *y_ui;

  MorphGrid          *morph_grid;
  MorphGridWidget    *grid_widget;

  TypeOperatorFilter  input_op_filter;

  ComboBoxOperator   *op_combobox;

public:
  MorphGridView (MorphGrid *morph_grid, MorphPlanWindow *morph_plan_window);

public slots:
  void on_size_changed();
  void on_selection_changed();
  void on_operator_changed();
  void on_plan_changed();
};

}

#endif
