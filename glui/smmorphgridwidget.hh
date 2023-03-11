// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MORPH_GRID_WIDGET_HH
#define SPECTMORPH_MORPH_GRID_WIDGET_HH

#include "smmorphgrid.hh"
#include "smwidget.hh"
#include "smvoicestatus.hh"

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

  Property& prop_x_morphing;
  Property& prop_y_morphing;

  bool move_controller = false;

  static constexpr double VOICE_RADIUS = 6;
  Point prop_to_pixel (double x, double y);

public:
  MorphGridWidget (Widget *parent, MorphGrid *morph_grid, MorphGridView *morph_grid_view);

  void draw (const DrawEvent& devent) override;
  void redraw_voices();

  void mouse_press (const MouseEvent& event) override;
  void mouse_release (const MouseEvent& event) override;
  void mouse_move (const MouseEvent& event) override;

/* slots: */
  void on_grid_params_changed();
  void on_voice_status_changed (VoiceStatus *voice_status);

/* signals: */
  Signal<> signal_selection_changed;
  Signal<> signal_grid_params_changed;
};

}

#endif

