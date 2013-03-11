// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_VIEW_HH
#define SPECTMORPH_MORPH_GRID_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphgrid.hh"

#include <QSpinBox>

namespace SpectMorph
{

class MorphGridView : public MorphOperatorView
{
  Q_OBJECT

  QSpinBox         *width_spinbox;
  QSpinBox         *height_spinbox;

  MorphGrid        *morph_grid;
public:
  MorphGridView (MorphGrid *morph_grid, MorphPlanWindow *morph_plan_window);

public slots:
  void on_size_changed();
};

}

#endif
