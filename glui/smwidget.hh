// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WIDGET_HH
#define SPECTMORPH_WIDGET_HH

namespace SpectMorph
{

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

}

#endif
