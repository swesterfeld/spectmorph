// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_WINDOW_HH
#define SPECTMORPH_WINDOW_HH

#include "smwidget.hh"
#include "pugl/pugl.h"
#include "smnativefiledialog.hh"
#include <memory>
#include <functional>
#include <map>

namespace SpectMorph
{

struct CairoGL;
class NativeFileDialog;
struct Menu;
class Shortcut;
class EventLoop;

class Window : public Widget
{
protected:
  struct UpdateRegion
  {
    Widget         *widget = nullptr;
    Rect            region;
    UpdateStrategy  update_strategy = UPDATE_MERGE;
  };
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
  bool                      keyboard_focus_release_on_click = false;
  Widget                   *dialog_widget = nullptr;
  std::unique_ptr<Window>   popup_window;
  double                    global_scale;
  std::vector<UpdateRegion> update_regions;
  bool                      update_full_redraw = true;
  bool                      debug_update_region = false;
  EventLoop                *m_event_loop = nullptr;
  double                    last_click_time = 0;
  unsigned                  last_click_button = 0;
  Point                     last_click_pos;
  unsigned                  mouse_buttons_pressed = 0;

  std::vector<Shortcut *>   shortcuts;

  std::function<void()>     m_close_callback;

  Widget *find_widget_xy (double ex, double ey);
  void on_button_event (const PuglEventButton& event);
  void on_motion_event (const PuglEventMotion& event);
  void on_scroll_event (const PuglEventScroll& event);
  void on_key_event (const PuglEventKey& event);
  void on_expose_event (const PuglEventExpose& event);
  void on_close_event (const PuglEventClose& event);
  void on_configure_event (const PuglEventConfigure& event);

  struct RedrawParams {
    Rect                                update_region;
    bool                                full_redraw;
    std::map<Widget *, Rect>            merged_regions;
    std::vector<std::vector<Widget *>>  visible_widgets_by_layer;
  };
  void collect_widgets_for_redraw (RedrawParams& redraw_params, Widget *widget, int layer);
  void redraw_update_region (const RedrawParams& params);

public:
  Window (EventLoop& event_loop, const std::string& title, int width, int height, PuglNativeWindow parent = 0, bool resize = false, PuglNativeWindow transient_parent = 0);
  virtual ~Window();

  std::vector<Widget *> crawl_widgets();
  void on_event (const PuglEvent *event);
  void on_resize (int *width, int *height);
  void process_events();
  EventLoop *event_loop() const;
  void show();
  void open_file_dialog (const std::string& title, const FileDialogFormats& formats, std::function<void(std::string)> callback);
  void save_file_dialog (const std::string& title, const FileDialogFormats& formats, std::function<void(std::string)> callback);
  void on_file_selected (const std::string& filename);
  void need_update (Widget *widget, const Rect *changed_rect, UpdateStrategy update_strategy);
  void on_widget_deleted (Widget *widget);
  void set_menu_widget (Widget *widget);
  void set_keyboard_focus (Widget *widget, bool release_on_click = false);
  bool has_keyboard_focus (Widget *widget);
  void set_dialog_widget (Widget *widget);
  void set_close_callback (const std::function<void()>& callback);
  void set_popup_window (Window *window);
  void add_shortcut (Shortcut *shortcut);
  void remove_shortcut (Shortcut *shortcut);
  Window *window() override;
  PuglNativeWindow native_window();

  void fill_zoom_menu (Menu *menu);
  void set_gui_scaling (double s);
  double gui_scaling();

  void get_scaled_size (int *w, int *h);

  Signal<> signal_update_size;
};

/* helper to remove all nullptr entries from a vector */
template<class T>
void
cleanup_null (std::vector<T *>& vec)
{
  std::vector<T *> new_vec;
  for (auto object : vec)
    if (object)
      new_vec.push_back (object);

  if (vec.size() > new_vec.size())
    vec = new_vec;
}

}

#endif
