// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WINDOW_HH
#define SPECTMORPH_WINDOW_HH

#include "smwidget.hh"
#include "pugl/pugl.h"
#include <memory>

namespace SpectMorph
{

struct CairoGL;

struct Window : public Widget
{
  PuglView                 *view;
  std::unique_ptr<CairoGL>  cairo_gl;
  bool                      quit;
  bool                      draw_grid;
  Widget                   *mouse_widget;
  Widget                   *enter_widget;
  double                    global_scale;

  Window (int width, int height, PuglNativeWindow parent = 0, bool resize = false);
  virtual ~Window();

  std::vector<Widget *> crawl_widgets();
  void on_display();
  void on_event (const PuglEvent *event);
  void wait_for_event();
  void process_events();
  void show();
  void draw (cairo_t *cr) override;
  void update() override;
  void on_dead_child (Widget *widget) override;
};

}

#endif
