// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_FOLD_BUTTON_HH
#define SPECTMORPH_FOLD_BUTTON_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

struct FoldButton : public Widget
{
  bool        highlight = false;
  bool        pressed = false;
  bool        folded;

  Signal<> signal_clicked;

  FoldButton (Widget *parent, bool folded) :
    Widget (parent),
    folded (folded)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 4;

    if (highlight)
      cairo_set_source_rgb (cr, 0.9, 0.9, 0.9);
    else
      cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);

    if (!folded) /* draw triangle */
      {
        cairo_move_to (cr, space, space);
        cairo_line_to (cr, width - space, space);
        cairo_line_to (cr, width / 2, space + (width - 2 * space) * 0.8);
      }
    else
      {
        cairo_move_to (cr, space, space);
        cairo_line_to (cr, space, height - space);
        cairo_line_to (cr, space + (height - 2 * space) * 0.8, height / 2);
      }

    cairo_close_path (cr);
    cairo_stroke_preserve (cr);
    cairo_fill (cr);
  }
  void
  enter_event() override
  {
    highlight = true;
  }
  void
  mouse_press (double x, double y) override
  {
    pressed = true;
  }
  void
  mouse_release (double x, double y) override
  {
    if (pressed && x >= 0 && y >= 0 && x < width && y < height)
      {
        folded = !folded;
        signal_clicked();
      }
  }
  void
  leave_event() override
  {
    highlight = false;
    pressed = false;
  }
};

}

#endif
