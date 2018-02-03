// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwindow.hh"
#include "pugl/cairo_gl.h"
#include <string.h>

using namespace SpectMorph;

using std::vector;
using std::min;

struct SpectMorph::CairoGL
{
private:
  PuglCairoGL pugl_cairo_gl;
  cairo_surface_t *surface;
  int width;
  int height;

public:
  cairo_t *cr;

  CairoGL (int width, int height) :
    width (width), height (height)
  {
    memset (&pugl_cairo_gl, 0, sizeof (pugl_cairo_gl));

    surface = pugl_cairo_gl_create (&pugl_cairo_gl, width, height, 4);
    cr = cairo_create (surface);
  }
  ~CairoGL()
  {
    cairo_destroy (cr);
    cairo_surface_destroy (surface);

    pugl_cairo_gl_free (&pugl_cairo_gl);
  }
  void
  configure()
  {
    pugl_cairo_gl_configure (&pugl_cairo_gl, width, height);
  }
  void
  draw()
  {
    pugl_cairo_gl_draw (&pugl_cairo_gl, width, height);
  }
};

static void
on_event (PuglView* view, const PuglEvent* event)
{
  Window *window = reinterpret_cast<Window *> (puglGetHandle (view));

  window->on_event (event);
}

Window::Window (int width, int height, PuglNativeWindow win_id, bool resize) :
  Widget (nullptr, 0, 0, width, height),
  quit (false),
  draw_grid (false),
  mouse_widget (nullptr),
  enter_widget (nullptr),
  global_scale (1.0)
{
  view = puglInit (nullptr, nullptr);

  puglInitWindowClass (view, "PuglTest");
  puglInitWindowSize (view, width, height);
  //puglInitWindowMinSize (view, 256, 256);
  puglInitResizable (view, resize);
  puglIgnoreKeyRepeat (view, false);
  if (win_id)
    puglInitWindowParent (view, win_id);
  puglCreateWindow (view, "Pugl Test");

  puglSetHandle (view, this);
  puglSetEventFunc (view, ::on_event);

  cairo_gl.reset (new CairoGL (width, height));

  puglEnterContext (view);
  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LESS);
  glClearColor (0.4f, 0.4f, 0.4f, 1.0f);
  cairo_gl->configure();
  printf ("OpenGL Version: %s\n",(const char*) glGetString(GL_VERSION));
  fflush (stdout);
  puglLeaveContext(view, false);
}

Window::~Window()
{
  puglDestroy (view);
}

void
Window::on_dead_child (Widget *child)
{
  /* cheap weak pointer emulation */
  if (mouse_widget == child)
    mouse_widget = nullptr;
  if (enter_widget == child)
    enter_widget = nullptr;
}

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

vector<Widget *>
Window::crawl_widgets()
{
  return ::crawl_widgets ({ this });
}

void
Window::on_display()
{
  // glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (auto w : crawl_widgets())
    {
      cairo_t *cr = cairo_gl->cr;

      cairo_save (cr);

      cairo_scale (cr, global_scale, global_scale);
      // local coordinates
      cairo_translate (cr, w->x, w->y);
      cairo_rectangle (cr, 0, 0, w->width, w->height);
      cairo_clip (cr);

      if (draw_grid && w == enter_widget)
        w->debug_fill (cr);

      w->draw (cr);
      cairo_restore (cr);
    }

  cairo_gl->draw();
}

void
Window::draw (cairo_t *cr)
{
  Widget::draw (cr);

  if (draw_grid)
    {
      cairo_set_source_rgb (cr, 0, 0, 0);
      cairo_set_line_width (cr, 0.5);

      for (double x = 8; x < width; x += 8)
        {
          for (double y = 8; y < height; y += 8)
            {
              cairo_move_to (cr, x - 2, y);
              cairo_line_to (cr, x + 2, y);
              cairo_move_to (cr, x, y - 2);
              cairo_line_to (cr, x, y + 2);
            }
        }
      cairo_stroke (cr);
    }
}

void
Window::process_events()
{
  puglProcessEvents (view);
}

static void
dump_event (const PuglEvent *event)
{
  switch (event->type)
    {
      case PUGL_NOTHING:            printf ("Event: nothing\n");
        break;
      case PUGL_BUTTON_PRESS:       printf ("Event: button press\n");
        break;
      case PUGL_BUTTON_RELEASE:     printf ("Event: button release\n");
        break;
      case PUGL_CONFIGURE:          printf ("Event: configure w%f h%f\n", event->configure.width, event->configure.height);
        break;
      case PUGL_EXPOSE:             printf ("Event: expose x%f y%f w%f h%f\n", event->expose.x, event->expose.y, event->expose.width, event->expose.height);
        break;
      case PUGL_CLOSE:              printf ("Event: close\n");
        break;
      case PUGL_KEY_PRESS:          printf ("Event: key press %c\n", event->key.character);
        break;
      case PUGL_KEY_RELEASE:        printf ("Event: key release\n");
        break;
      case PUGL_ENTER_NOTIFY:       printf ("Event: enter\n");
        break;
      case PUGL_LEAVE_NOTIFY:       printf ("Event: leave\n");
        break;
      case PUGL_MOTION_NOTIFY:      printf ("Event: motion\n");
        break;
      case PUGL_SCROLL:             printf ("Event: scroll\n");
        break;
      case PUGL_FOCUS_IN:           printf ("Event: focus in\n");
        break;
      case PUGL_FOCUS_OUT:          printf ("Event: focus out\n");
        break;
    }
}

void
Window::on_event (const PuglEvent* event)
{
  if (0)
    dump_event (event);

  double ex, ey; /* global scale translated */
  switch (event->type)
    {
      case PUGL_BUTTON_PRESS:
        ex = event->button.x / global_scale;
        ey = event->button.y / global_scale;
        for (auto w : crawl_widgets())
          {
            if (ex >= w->x &&
                ey >= w->y &&
                ex < w->x + w->width &&
                ey < w->y + w->height)
              {
                w->mouse_press (ex - w->x, ey - w->y);
                mouse_widget = w;
              }
          }
        puglPostRedisplay (view);
        break;
      case PUGL_BUTTON_RELEASE:
        ex = event->button.x / global_scale;
        ey = event->button.y / global_scale;
        if (mouse_widget)
          {
            Widget *w = mouse_widget;
            w->mouse_release (ex - w->x, ey - w->y);
            mouse_widget = nullptr;
          }
        puglPostRedisplay (view);
        break;
      case PUGL_MOTION_NOTIFY:
        ex = event->motion.x / global_scale;
        ey = event->motion.y / global_scale;
        if (mouse_widget) /* user interaction with one widget */
          {
            Widget *w = mouse_widget;
            w->motion (ex - w->x, ey - w->y);
          }
        else
          {
            for (auto w : crawl_widgets()) /* no specific widget, search for match */
              {
                if (ex >= w->x &&
                    ey >= w->y &&
                    ex < w->x + w->width &&
                    ey < w->y + w->height)
                  {
                    if (enter_widget)
                      enter_widget->leave_event();
                    enter_widget = w;
                    w->enter_event();
                    w->motion (ex - w->x, ey - w->y);
                  }
              }
          }
        puglPostRedisplay (view);
        break;
      case PUGL_KEY_PRESS:
        if (event->key.character == 'g')
          draw_grid = !draw_grid;
        update();
        break;
      case PUGL_CLOSE:
        quit = true;
        break;
      case PUGL_EXPOSE:
        on_display();
        break;
      case PUGL_CONFIGURE:
        global_scale = min (event->configure.width, event->configure.height) / 360.;
        cairo_gl.reset (new CairoGL (event->configure.width, event->configure.height));
        puglEnterContext (view);
        cairo_gl->configure();
        puglLeaveContext (view, false);
        break;
      default:
        break;
    }
}

void
Window::show()
{
  // puglPostRedisplay (view);
  puglShowWindow (view);
}

void
Window::update()
{
  puglPostRedisplay (view);
}

void
Window::wait_for_event()
{
  puglWaitForEvent (view);
}
