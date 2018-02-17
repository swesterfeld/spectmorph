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

class ScrollBar : public Widget
{
  double page_size;
public:
  double pos;
  double old_pos;
  double mouse_y;
  double mouse_x;
  bool mouse_down = false;
  bool highlight = false;
  Rect clickable_rect;

  Orientation orientation;

  Signal<double> signal_position_changed;

  ScrollBar (Widget *parent, double new_page_size, Orientation orientation) :
    Widget (parent),
    pos (0),
    orientation (orientation)
  {
    page_size = sm_bound<double> (0, new_page_size, 1);
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    const double space = 2;

    const double rwidth = width - 2 * space;
    const double rheight = height - 2 * space;

    if (enabled())
      cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 1);
    else
      cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 1);
    du.round_box (space, space, rwidth, rheight, 1, 5, true);

    if (enabled())
      {
        if (highlight || mouse_down)
          cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1);
        else
          cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 1);
      }
    else
      cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 1);

    if (orientation == Orientation::HORIZONTAL)
      clickable_rect = Rect (space + pos * rwidth, space, rwidth * page_size, rheight);
    else
      clickable_rect = Rect (space, space + pos * rheight, rwidth, rheight * page_size);

    du.round_box (clickable_rect, 1, 5, true);
  }
  void
  mouse_press (double x, double y) override
  {
    if (clickable_rect.contains (x, y))
      {
        mouse_down = true;
        mouse_y = y;
        mouse_x = x;
        old_pos = pos;
      }
  }
  void
  motion (double x, double y) override
  {
    highlight = clickable_rect.contains (x, y);

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

