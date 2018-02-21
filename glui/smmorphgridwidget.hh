// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_WIDGET_HH
#define SPECTMORPH_MORPH_GRID_WIDGET_HH

#include "smmorphgrid.hh"
#include "smwidget.hh"

namespace SpectMorph
{

class MorphGridWidget : public Widget
{
  MorphGrid *morph_grid;

public:
  MorphGridWidget (Widget *parent, MorphGrid *morph_grid);

/* slots: */
  void on_plan_changed();

/* signals: */
  Signal<> signal_selection_changed;
};

}

#endif

