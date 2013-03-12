// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_VIEW_HH
#define SPECTMORPH_MORPH_GRID_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphgrid.hh"
#include "smcomboboxoperator.hh"
#include "smmorphgridwidget.hh"

#include <QSpinBox>

namespace SpectMorph
{

class MorphGridView : public MorphOperatorView
{
  Q_OBJECT

  QSpinBox         *width_spinbox;
  QSpinBox         *height_spinbox;

  ComboBoxOperator *op_combobox;
  MorphGrid        *morph_grid;
  MorphGridWidget  *grid_widget;

public:
  MorphGridView (MorphGrid *morph_grid, MorphPlanWindow *morph_plan_window);

public slots:
  void on_size_changed();
  void on_selection_changed();
  void on_operator_changed();
};

}

#endif
