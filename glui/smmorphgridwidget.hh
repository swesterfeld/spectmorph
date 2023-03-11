// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_GRID_WIDGET_HH
#define SPECTMORPH_MORPH_GRID_WIDGET_HH

#include "smmorphgrid.hh"
#include "smwidget.hh"
#include "smmidisynth.hh"

namespace SpectMorph
{

class MorphGridView;
class MorphGridWidget : public Widget
{
  MorphGrid *morph_grid;

  std::vector<int> x_coord;
  std::vector<int> y_coord;
  std::vector<float> x_voice_values;
  std::vector<float> y_voice_values;
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
  void on_synth_notify_event (SynthNotifyEvent *ne);

/* signals: */
  Signal<> signal_selection_changed;
  Signal<> signal_grid_params_changed;
};

}

#endif

