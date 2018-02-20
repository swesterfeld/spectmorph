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

    Color bg_color;
    if (enabled())
      bg_color.set_rgb (0.5, 0.5, 0.5);
    else
      bg_color.set_rgb (0.3, 0.3, 0.3);
    du.round_box (space, space, rwidth, rheight, 1, 5, Color::null(), bg_color);

    Color fg_color;
    if (enabled())
      {
        if (highlight || mouse_down)
          fg_color.set_rgb (0.8, 0.8, 0.8);
        else
          fg_color.set_rgb (0.7, 0.7, 0.7);
      }
    else
      fg_color.set_rgb (0.5, 0.5, 0.5);

    if (orientation == Orientation::HORIZONTAL)
      clickable_rect = Rect (space + pos * rwidth, space, rwidth * page_size, rheight);
    else
      clickable_rect = Rect (space, space + pos * rheight, rwidth, rheight * page_size);

    du.round_box (clickable_rect, 1, 5, Color::null(), fg_color);
  }
  void
  mouse_press (double x, double y) override
  {
    if (clickable_rect.contains (x, y)) /* drag scroll bar */
      {
        mouse_down = true;
        mouse_y = y;
        mouse_x = x;
        old_pos = pos;
      }
    else /* click, not in scroll bar rect => jump one page */
      {
        double new_pos = pos;

        if (orientation == Orientation::HORIZONTAL)
          {
            if (x < clickable_rect.x())
              new_pos = pos - page_size;
            else if (x > clickable_rect.x() + clickable_rect.width())
              new_pos = pos + page_size;
          }
        else
          {
            if (y < clickable_rect.y())
              new_pos = pos - page_size;
            else if (y > clickable_rect.y() + clickable_rect.height())
              new_pos = pos + page_size;
          }

        new_pos = sm_bound (0.0, new_pos, 1 - page_size);
        if (pos != new_pos)
          {
            pos = new_pos;
            signal_position_changed (pos);
          }
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

