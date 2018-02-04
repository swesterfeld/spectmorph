// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SCROLLBAR_HH
#define SPECTMORPH_SCROLLBAR_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

struct ScrollBar : public Widget
{
  double page_size;
  double pos;
  double old_pos;
  double mouse_y;
  bool mouse_down = false;

  ScrollBar (Widget *parent, double page_size) :
    Widget (parent, 0, 0, 100, 100),
    page_size (page_size),
    pos (0)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;

    cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 1);
    du.round_box (space, 0, width - 2 * space, height, 1, 5, true);

    cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1);
    du.round_box (space, pos * height, width - 2 * space, height * page_size, 1, 5, true);
  }
  void
  mouse_press (double x, double y) override
  {
    mouse_down = true;
    mouse_y = y;
    old_pos = pos;
  }
  void
  motion (double x, double y) override
  {
    if (mouse_down)
      {
        pos = old_pos + (y - mouse_y) / height;
        if (pos > (1 - page_size))
          pos = 1 - page_size;
        if (pos < 0)
          pos = 0;
      }
  }
  void
  mouse_release (double mx, double my) override
  {
  }
};

}

#endif

