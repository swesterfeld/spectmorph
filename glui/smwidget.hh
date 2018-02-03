// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WIDGET_HH
#define SPECTMORPH_WIDGET_HH

#include <vector>
#include <cairo.h>
#include <stdio.h>

namespace SpectMorph
{

struct Widget
{
protected:
  void remove_child (Widget *child);

public:
  Widget *parent;
  double x, y, width, height;

  std::vector<Widget *> children;

  void debug_fill (cairo_t *cr)
  {
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_set_source_rgb (cr, 0.6, 0.6, 0.6);
    cairo_fill (cr);
  }

  Widget (Widget *parent, double x, double y, double width, double height);
  virtual ~Widget();

  virtual void
  draw (cairo_t *cr)
  {
    cairo_set_source_rgb (cr, 0.2, 0.2, 0.2);
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
  virtual void
  update()
  {
    if (parent)
      parent->update();
  }
  virtual void
  on_dead_child (Widget *widget)
  {
  }
};

}

#endif
