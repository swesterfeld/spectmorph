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

static std::vector<Widget *> widgets; // should be a tree

static cairo_t *cr = 0;
static Widget  *mouse_widget = 0;
static Widget  *enter_widget = 0;

static void
onDisplay(PuglView* view)
{
  const int view_width = 512;
  const int view_height = 512;

  // glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (auto w : widgets)
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
        for (auto w : widgets)
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
            for (auto w : widgets) /* no specific widget, search for match */
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

  Widget *background = new Widget (0, 0, 512, 512);

  Label *l_attack = new Label (20, 100, 150, 40, "Attack");
  Slider *s_attack = new Slider (200, 100, 170, 40, 0.0);
  Label *l_attack_value = new Label (400, 100, 80, 40, "50%");

  Label *l_sustain = new Label (20, 150, 150, 40, "Sustain");
  Slider *s_sustain = new Slider (200, 150, 170, 40, 1.0);
  Label *l_sustain_value = new Label (400, 150, 80, 40, "50%");

  s_attack->set_callback ([=](float value) { l_attack_value->text = std::to_string((int) (value * 100 + 0.5)) + "%"; });
  s_sustain->set_callback ([=](float value) { l_sustain_value->text = std::to_string((int) (value * 100 + 0.5)) + "%"; });

  widgets.push_back (background);
  widgets.push_back (s_attack);
  widgets.push_back (s_sustain);
  widgets.push_back (l_attack);
  widgets.push_back (l_sustain);
  widgets.push_back (l_attack_value);
  widgets.push_back (l_sustain_value);
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

  Widget background (0, 0, 512, 512);

  Label l_attack (20, 100, 150, 40, "Attack");
  Slider s_attack (200, 100, 170, 40, 0.0);
  Label l_attack_value (400, 100, 80, 40, "50%");

  Label l_sustain (20, 150, 150, 40, "Sustain");
  Slider s_sustain (200, 150, 170, 40, 1.0);
  Label l_sustain_value (400, 150, 80, 40, "50%");

  s_attack.set_callback ([&](float value) { l_attack_value.text = std::to_string((int) (value * 100 + 0.5)) + "%"; });
  s_sustain.set_callback ([&](float value) { l_sustain_value.text = std::to_string((int) (value * 100 + 0.5)) + "%"; });

  widgets.push_back (&background);
  widgets.push_back (&s_attack);
  widgets.push_back (&s_sustain);
  widgets.push_back (&l_attack);
  widgets.push_back (&l_sustain);
  widgets.push_back (&l_attack_value);
  widgets.push_back (&l_sustain_value);

  while (!quit) {
    puglWaitForEvent (view);
    puglProcessEvents (view);
  }

  pugl_cairo_gl_free (&cairo_gl);
  puglDestroy(view);
}
#endif
