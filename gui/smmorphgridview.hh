// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_VIEW_HH
#define SPECTMORPH_MORPH_GRID_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphgrid.hh"

#include <QComboBox>

namespace SpectMorph
{

class MorphGridView : public MorphOperatorView
{
  Q_OBJECT

  MorphGrid        *morph_grid;

public:
  MorphGridView (MorphGrid *morph_grid, MorphPlanWindow *morph_plan_window);
};

}

#endif
