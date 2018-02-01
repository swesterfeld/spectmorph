#include <stdio.h>
#include "pugl/gl.h"
#if !__APPLE__
#include "GL/glext.h"
#endif
#include "pugl/pugl.h"
#include "pugl/cairo_gl.h"
#include <unistd.h>
#include <math.h>
#include <string>
#include <vector>
#include <functional>

#include "smwidget.hh"
#include "smlabel.hh"
#include "smslider.hh"

static bool quit = false;
static PuglCairoGL cairo_gl = { 0, };

using namespace SpectMorph;

static Widget *static_main_window;
using std::vector;

static cairo_t *cr = 0;
static Widget  *mouse_widget = 0;
static Widget  *enter_widget = 0;

static vector<Widget *>
crawl_widgets (const vector<Widget *>& widgets)
{
  vector<Widget *> all_widgets;

  for (auto w : widgets)
    {
      all_widgets.push_back (w);
      auto c_result = crawl_widgets (w->children);
      all_widgets.insert (all_widgets.end(), c_result.begin(), c_result.end());
    }
  return all_widgets;
}

static vector<Widget *>
crawl_widgets()
{
  return crawl_widgets ({ static_main_window });
}

static void
onDisplay(PuglView* view)
{
  const int view_width = 512;
  const int view_height = 512;

  // glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (auto w : crawl_widgets())
    {
      cairo_save (cr);

      // local coordinates
      cairo_translate (cr, w->x, w->y);
      cairo_rectangle (cr, 0, 0, w->width, w->height);
      cairo_clip (cr);

      w->draw (cr);
      cairo_restore (cr);
    }

  pugl_cairo_gl_draw (&cairo_gl, view_width, view_height);
}

static void
onEvent (PuglView* view, const PuglEvent* event)
{
  switch (event->type)
    {
      case PUGL_BUTTON_PRESS:
        for (auto w : crawl_widgets())
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
            for (auto w : crawl_widgets()) /* no specific widget, search for match */
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
        onDisplay (view);
        break;
      default:
        break;
    }
}

class MainWindow : public Widget
{
public:
  MainWindow (int width, int height) :
    Widget (nullptr, 0, 0, width, height)
  {
    Label  *l_attack = new Label (this, 20, 100, 150, 40, "Attack");
    Slider *s_attack = new Slider (this, 200, 100, 170, 40, 0.0);
    Label  *l_attack_value = new Label (this, 400, 100, 80, 40, "50%");

    Label  *l_sustain = new Label (this, 20, 150, 150, 40, "Sustain");
    Slider *s_sustain = new Slider (this, 200, 150, 170, 40, 1.0);
    Label  *l_sustain_value = new Label (this, 400, 150, 80, 40, "50%");

    new Label (this, 0, 200, 512, 300, " --- main window --- ");

    s_attack->set_callback ([=](float value) { l_attack_value->text = std::to_string((int) (value * 100 + 0.5)) + "%"; });
    s_sustain->set_callback ([=](float value) { l_sustain_value->text = std::to_string((int) (value * 100 + 0.5)) + "%"; });

    static_main_window = this;
  }
};

#if VST_PLUGIN
static PuglView* global_vst_view;

void
plugin_open (uintptr_t win_id)
{
  PuglView* view = puglInit (NULL, NULL);

  global_vst_view = view;

  puglInitWindowClass(view, "PuglTest");
  puglInitWindowSize(view, 512, 512);
  puglInitWindowMinSize(view, 256, 256);
  puglInitWindowParent (view, win_id);
  puglInitResizable(view, false);
  puglIgnoreKeyRepeat(view, false);

  puglSetEventFunc(view, onEvent);

  puglCreateWindow (view, "Pugl Test");

  int view_width  = 512;
  int view_height = 512;
  cairo_surface_t* surface = pugl_cairo_gl_create(&cairo_gl, view_width, view_height, 4);
  cr = cairo_create (surface);

  puglEnterContext(view);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
  pugl_cairo_gl_configure(&cairo_gl, view_width, view_height);
  printf ("OpenGL Version: %s\n", (const char*) glGetString(GL_VERSION));
  fflush (stdout);
  puglLeaveContext(view, false);

  puglPostRedisplay (view);
  puglShowWindow (view);

  Widget *main_window = new MainWindow (512, 512);
}
#else
int
main()
{
  PuglView* view = puglInit (NULL, NULL);

  puglInitWindowClass(view, "PuglTest");
  puglInitWindowSize(view, 512, 512);
  puglInitWindowMinSize(view, 256, 256);
  puglInitResizable(view, false);
  puglIgnoreKeyRepeat(view, false);

  puglSetEventFunc(view, onEvent);

  puglCreateWindow (view, "Pugl Test");

  int view_width  = 512;
  int view_height = 512;
  cairo_surface_t* surface = pugl_cairo_gl_create(&cairo_gl, view_width, view_height, 4);
  cr = cairo_create (surface);

  puglEnterContext(view);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
  pugl_cairo_gl_configure(&cairo_gl, view_width, view_height);
  printf ("OpenGL Version: %s\n",(const char*) glGetString(GL_VERSION));
  fflush (stdout);
  puglLeaveContext(view, false);

  puglPostRedisplay (view);
  puglShowWindow (view);

  MainWindow *window = new MainWindow (512, 512);

  while (!quit) {
    puglWaitForEvent (view);
    puglProcessEvents (view);
  }

  delete window;
  pugl_cairo_gl_free (&cairo_gl);
  puglDestroy(view);
}
#endif
