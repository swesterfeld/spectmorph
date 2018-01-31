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

static bool quit = false;
static PuglCairoGL cairo_gl = { 0, };

struct Widget
{
  double x, y, width, height;

  void debug_fill (cairo_t *cr)
  {
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_set_source_rgb (cr, 0.6, 0.6, 0.6);
    cairo_fill (cr);
  }

  Widget (double x, double y, double width, double height) :
    x (x), y (y), width (width), height (height)
  {
  }
  virtual void
  draw (cairo_t *cr)
  {
    cairo_set_source_rgb (cr, 0.4, 0.4, 0.4);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);
  }

  virtual void motion (double x, double y)
  {
  }
  virtual void
  mouse_press (double x, double y)
  {
  }
  virtual void
  mouse_release (double x, double y)
  {
  }
  virtual void
  enter_event()
  {
  }
  virtual void
  leave_event()
  {
  }
};

static std::vector<Widget *> widgets; // should be a tree

struct Label : public Widget
{
  std::string text;

  Label (double x, double y, double w, double h, const std::string& text) :
    Widget (x, y, w, h),
    text (text)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    //debug_fill (cr);

    // draw label
    cairo_set_font_size (cr, 24.0);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, text.c_str(), &extents);

    cairo_move_to (cr, (width / 2) - extents.width / 2, (height / 2) + extents.height / 2);
    cairo_set_source_rgba (cr, 1, 1, 1, 1);
    cairo_show_text (cr, text.c_str());
  }
};

struct Slider : public Widget
{
  double value;
  bool highlight = false;
  bool mouse_down = false;
  bool enter = false;
  std::function<void(float)> m_callback;

  Slider (double x, double y, double width, double height, double value)
    : Widget (x, y, width, height),
      value (value)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    /*if (enter)
      debug_fill (cr); */

    double H = 8; // height of slider thing
    double C = 12;
    double value_pos = C + (width - C * 2) * value;

    cairo_rectangle (cr, C, height / 2 - H / 2, value_pos, H);
    cairo_set_source_rgb (cr, 0.1, 0.9, 0.1);
    cairo_fill (cr);

    cairo_rectangle (cr, value_pos, height / 2 - H / 2, (width - C - value_pos), H);
    cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
    cairo_fill (cr);

    if (highlight || mouse_down)
      cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    else
      cairo_set_source_rgb (cr, 0.8, 0.8, 0.8);
    cairo_arc (cr, value_pos, height / 2, C, 0, 2 * M_PI);
    cairo_fill (cr);
  }
  bool
  in_circle (double x, double y)
  {
    double C = 12;
    double value_pos = C + (width - C * 2) * value;

    double dx = value_pos - x;
    double dy = height / 2 - y;
    double dist = sqrt (dx * dx + dy * dy);

    return (dist < C);
  }
  void
  motion (double x, double y) override
  {
    highlight = in_circle (x, y);
    if (mouse_down)
      {
        double C = 12;
        value = (x - C) / (width - C * 2);
        if (value < 0)
          value = 0;
        if (value > 1)
          value = 1;
        if (m_callback)
          m_callback (value);
      }
  }
  void
  set_callback (const std::function<void(float)> &callback)
  {
    m_callback = callback;
  }
  void
  mouse_press (double x, double y) override
  {
    if (in_circle (x, y))
      {
        mouse_down = true;
      }
  }
  void
  mouse_release (double x, double y) override
  {
    mouse_down = false;
  }
  void
  enter_event() override
  {
    enter = true;
  }
  void
  leave_event() override
  {
    enter = false;
    highlight = false;
  }
};

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
