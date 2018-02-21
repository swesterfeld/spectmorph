// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridwidget.hh"

using namespace SpectMorph;

MorphGridWidget::MorphGridWidget (Widget *parent, MorphGrid *morph_grid) :
  Widget (parent),
  morph_grid (morph_grid)
{
  set_background_color (Color (0.4, 0.4, 0.4));
}
