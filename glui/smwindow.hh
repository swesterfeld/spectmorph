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
class NativeFileDialog;

struct Window : public Widget
{
protected:
  PuglView                 *view;
  std::unique_ptr<CairoGL>  cairo_gl;
  bool                      draw_grid;
  bool                      have_file_dialog = false;
  std::function<void(std::string)> file_dialog_callback;
  std::unique_ptr<NativeFileDialog> native_file_dialog;
  Widget                   *mouse_widget = nullptr;
  Widget                   *enter_widget = nullptr;
  Widget                   *menu_widget = nullptr;
  Widget                   *keyboard_focus_widget = nullptr;
  Widget                   *dialog_widget = nullptr;
  double                    global_scale;
  bool                      auto_redraw;
  Rect                      update_region;
  bool                      update_full_redraw = false;

  std::function<void()>     m_close_callback;

  Widget *find_widget_xy (double ex, double ey);

public:
  Window (const std::string& title, int width, int height, PuglNativeWindow parent = 0, bool resize = false);
  virtual ~Window();

  std::vector<Widget *> crawl_widgets();
  void on_display();
  void on_event (const PuglEvent *event);
  void on_resize (int *width, int *height);
  void wait_for_event();
  void wait_event_fps();
  void process_events();
  void show();
  void open_file_dialog (const std::string& title, const std::string& filter, const std::string& filter_title, std::function<void(std::string)> callback);
  void save_file_dialog (const std::string& title, const std::string& filter, const std::string& filter_title, std::function<void(std::string)> callback);
  void on_file_selected (const std::string& filename);
  void need_update (Widget *widget);
  void on_widget_deleted (Widget *widget);
  void set_menu_widget (Widget *widget);
  void set_keyboard_focus (Widget *widget);
  void set_dialog_widget (Widget *widget);
  void set_close_callback (const std::function<void()>& callback);
  Window *window() override;
  PuglNativeWindow native_window();
  void set_gui_scaling (double s);
  double gui_scaling();

  void get_scaled_size (int *w, int *h);

  Signal<> signal_update_size;
};

}

#endif
