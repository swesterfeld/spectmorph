// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_GRID_WIDGET_HH
#define SPECTMORPH_MORPH_GRID_WIDGET_HH

#include "smmorphgrid.hh"
#include "smwidget.hh"

namespace SpectMorph
{

class MorphGridView;
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
  MorphGridWidget (Widget *parent, MorphGrid *morph_grid, MorphGridView *morph_grid_view);

  void draw (const DrawEvent& devent) override;

  void mouse_press (const MouseEvent& event) override;
  void mouse_release (const MouseEvent& event) override;
  void mouse_move (const MouseEvent& event) override;

/* slots: */
  void on_grid_params_changed();

/* signals: */
  Signal<> signal_selection_changed;
  Signal<> signal_grid_params_changed;
};

}

#endif

