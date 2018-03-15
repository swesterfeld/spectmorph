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

  std::vector<int> x_coord;
  std::vector<int> y_coord;
  double start_x = 0;
  double start_y = 0;
  double end_x = 0;
  double end_y = 0;

  bool move_controller = false;

public:
  MorphGridWidget (Widget *parent, MorphGrid *morph_grid);

  void draw (cairo_t *cr) override;

  void mouse_press (double x, double y) override;
  void mouse_release (double x, double y) override;
  void motion (double x, double y) override;

/* slots: */
  void on_plan_changed();

/* signals: */
  Signal<> signal_selection_changed;
};

}

#endif

