// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridview.hh"
#include "smmorphplan.hh"

using namespace SpectMorph;

MorphGridView::MorphGridView (MorphGrid *morph_grid, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_grid, morph_plan_window),
  morph_grid (morph_grid)
{
}
