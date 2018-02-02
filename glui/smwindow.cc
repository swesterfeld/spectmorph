// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwindow.hh"
#include <string.h>

using namespace SpectMorph;

using std::vector;

Window::Window (int width, int height, PuglNativeWindow win_id) :
  Widget (nullptr, 0, 0, width, height)
{
  view = puglInit (nullptr, nullptr);

  puglInitWindowClass(view, "PuglTest");
  puglInitWindowSize(view, width, height);
  //puglInitWindowMinSize(view, 256, 256);
  puglInitResizable(view, false);
  puglIgnoreKeyRepeat(view, false);
  if (win_id)
    puglInitWindowParent (view, win_id);
  puglCreateWindow (view, "Pugl Test");

  memset (&cairo_gl, 0, sizeof (cairo_gl));
  cairo_surface_t* surface = pugl_cairo_gl_create (&cairo_gl, width, height, 4);
  cr = cairo_create (surface);

  puglEnterContext (view);
  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LESS);
  glClearColor (0.4f, 0.4f, 0.4f, 1.0f);
  pugl_cairo_gl_configure (&cairo_gl, width, height);
  printf ("OpenGL Version: %s\n",(const char*) glGetString(GL_VERSION));
  fflush (stdout);
  puglLeaveContext(view, false);
}

Window::~Window()
{
  pugl_cairo_gl_free (&cairo_gl);
  puglDestroy (view);
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
