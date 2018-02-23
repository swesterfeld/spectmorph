// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WINDOW_HH
#define SPECTMORPH_WINDOW_HH

#include "smwidget.hh"
#include "pugl/pugl.h"
#include <memory>
#include <functional>

namespace SpectMorph
{

struct CairoGL;

struct Window : public Widget
{
protected:
  PuglView                 *view;
  std::unique_ptr<CairoGL>  cairo_gl;
  bool                      draw_grid;
  Widget                   *mouse_widget;
  Widget                   *enter_widget;
  Widget                   *menu_widget;
  double                    global_scale;
  double                    scale_to_width;

  std::function<void()>     m_close_callback;

  Widget *find_widget_xy (double ex, double ey);

public:
  Window (int width, int height, PuglNativeWindow parent = 0, bool resize = false);
  virtual ~Window();

  std::vector<Widget *> crawl_widgets();
  void on_display();
  void on_event (const PuglEvent *event);
  void wait_for_event();
  void process_events();
  void show();
  void open_file_dialog (const std::string& title);
  void on_file_selected (const char *filename);
  void draw (cairo_t *cr) override;
  void update() override;
  void on_widget_deleted (Widget *widget);
  void set_menu_widget (Widget *widget);
  void set_close_callback (const std::function<void()>& callback);
  Window *window() override;
};

}

#endif
