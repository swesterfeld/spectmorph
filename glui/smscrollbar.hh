// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SCROLLBAR_HH
#define SPECTMORPH_SCROLLBAR_HH

#include "smdrawutils.hh"
#include "smmath.hh"

namespace SpectMorph
{

enum class
Orientation
{
  HORIZONTAL,
  VERTICAL
};

struct ScrollBar : public Widget
{
  double page_size;
  double pos;
  double old_pos;
  double mouse_y;
  double mouse_x;
  bool mouse_down = false;

  Orientation orientation;

  Signal<double> signal_position_changed;

  ScrollBar (Widget *parent, double page_size, Orientation orientation) :
    Widget (parent),
    page_size (page_size),
    pos (0),
    orientation (orientation)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;

    if (orientation == Orientation::HORIZONTAL)
      {
        cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 1);
        du.round_box (space, space, width - 2 * space, height - 2 * space, 1, 5, true);

        cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1);
        du.round_box (space + pos * width, space, width * page_size, height - 2 * space, 1, 5, true);
      }
    else
      {
        cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 1);
        du.round_box (space, space, width - 2 * space, height - 2 * space, 1, 5, true);

        cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1);
        du.round_box (space, pos * height, width - 2 * space, height * page_size, 1, 5, true);
      }
  }
  void
  mouse_press (double x, double y) override
  {
    mouse_down = true;
    mouse_y = y;
    mouse_x = x;
    old_pos = pos;
  }
  void
  motion (double x, double y) override
  {
    if (mouse_down)
      {
        if (orientation == Orientation::VERTICAL)
          pos = old_pos + (y - mouse_y) / height;
        else
          pos = old_pos + (x - mouse_x) / width;

        pos = sm_bound (0.0, pos, 1 - page_size);
        signal_position_changed (pos);
      }
  }
  void
  scroll (double dx, double dy) override
  {
    pos = sm_bound<double> (0, pos - 0.25 * page_size * dy, 1 - page_size);

    signal_position_changed (pos);
  }
  void
  mouse_release (double mx, double my) override
  {
    mouse_down = false;
  }
};

}

#endif

