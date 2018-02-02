#include <stdio.h>
#include "pugl/gl.h"
#if !__APPLE__
#include "GL/glext.h"
#endif
#include "pugl/pugl.h"
#include <unistd.h>
#include <math.h>
#include <string>
#include <vector>
#include <functional>

#include "smwidget.hh"
#include "smlabel.hh"
#include "smslider.hh"
#include "smwindow.hh"
#include "smmain.hh"

static bool quit = false;

using namespace SpectMorph;

static class MainWindow *static_main_window;

class MainWindow : public Window
{
public:
  MainWindow (int width, int height, PuglNativeWindow win_id = 0) :
    Window (width, height, win_id)
  {
    new Label (this, 20, 100, 150, 40, "Attack");
    Slider *s_attack = new Slider (this, 200, 100, 170, 40, 0.0);
    Label  *l_attack_value = new Label (this, 400, 100, 80, 40, "50%");

    new Label (this, 20, 150, 150, 40, "Sustain");
    Slider *s_sustain = new Slider (this, 200, 150, 170, 40, 1.0);
    Label  *l_sustain_value = new Label (this, 400, 150, 80, 40, "50%");

    new Label (this, 0, 200, 512, 300, " --- main window --- ");

    s_attack->set_callback ([=](float value) { l_attack_value->text = std::to_string((int) (value * 100 + 0.5)) + "%"; });
    s_sustain->set_callback ([=](float value) { l_sustain_value->text = std::to_string((int) (value * 100 + 0.5)) + "%"; });

    static_main_window = this;
  }
};

using std::vector;

static Widget  *mouse_widget = 0;
static Widget  *enter_widget = 0;

static void
onEvent (PuglView* view, const PuglEvent* event)
{
  switch (event->type)
    {
      case PUGL_BUTTON_PRESS:
        for (auto w : static_main_window->crawl_widgets())
          {
            if (event->button.x >= w->x &&
                event->button.y >= w->y &&
                event->button.x < w->x + w->width &&
                event->button.y < w->y + w->height)
              {
                w->mouse_press (event->button.x - w->x, event->button.y - w->y);
                mouse_widget = w;
              }
          }
        puglPostRedisplay (view);
        break;
      case PUGL_BUTTON_RELEASE:
        if (mouse_widget)
          {
            Widget *w = mouse_widget;
            w->mouse_release (event->button.x - w->x, event->button.y - w->y);
            mouse_widget = nullptr;
          }
        puglPostRedisplay (view);
        break;
      case PUGL_MOTION_NOTIFY:
        if (mouse_widget) /* user interaction with one widget */
          {
            Widget *w = mouse_widget;
            w->motion (event->motion.x - w->x, event->motion.y - w->y);
          }
        else
          {
            for (auto w : static_main_window->crawl_widgets()) /* no specific widget, search for match */
              {
                if (event->motion.x >= w->x &&
                    event->motion.y >= w->y &&
                    event->motion.x < w->x + w->width &&
                    event->motion.y < w->y + w->height)
                  {
                    if (enter_widget)
                      enter_widget->leave_event();
                    enter_widget = w;
                    w->enter_event();
                    w->motion (event->motion.x - w->x, event->motion.y - w->y);
                  }
              }
          }
        puglPostRedisplay (view);
        break;
      case PUGL_CLOSE:
        quit = true;
        break;
      case PUGL_EXPOSE:
        static_main_window->on_display();
        break;
      default:
        break;
    }
}

#if VST_PLUGIN
static PuglView* global_vst_view;

void
plugin_open (uintptr_t win_id)
{
  MainWindow *window = new MainWindow (512, 512, win_id);

  PuglView* view = window->view;
  global_vst_view = view;

  puglSetEventFunc(view, onEvent);

  puglPostRedisplay (view);
  puglShowWindow (view);

}
#else
int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  MainWindow *window = new MainWindow (512, 512);

  PuglView* view = window->view;

  puglSetEventFunc(view, onEvent);

  puglPostRedisplay (view);
  puglShowWindow (view);

  while (!quit) {
    puglWaitForEvent (view);
    puglProcessEvents (view);
  }

  delete window;
}
#endif
