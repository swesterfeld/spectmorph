// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_TOOL_BUTTON_HH
#define SPECTMORPH_TOOL_BUTTON_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

class ToolButton : public Widget
{
  bool        highlight = false;
  bool        pressed = false;
  char        symbol;

public:
  Signal<> signal_clicked;

  ToolButton (Widget *parent, char symbol = 0) :
    Widget (parent),
    symbol (symbol)
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

    if (symbol == 'v') /* draw triangle */
      {
        cairo_move_to (cr, space, space);
        cairo_line_to (cr, width - space, space);
        cairo_line_to (cr, width / 2, space + (width - 2 * space) * 0.8);

        cairo_close_path (cr);
        cairo_stroke_preserve (cr);
        cairo_fill (cr);
      }
    else if (symbol == '>')
      {
        cairo_move_to (cr, space, space);
        cairo_line_to (cr, space, height - space);
        cairo_line_to (cr, space + (height - 2 * space) * 0.8, height / 2);

        cairo_close_path (cr);
        cairo_stroke_preserve (cr);
        cairo_fill (cr);
      }
    else if (symbol == 'x')
      {
        cairo_move_to (cr, space, space);
        cairo_line_to (cr, width - space, height - space);

        cairo_move_to (cr, space, height - space);
        cairo_line_to (cr, width - space, space);

        cairo_set_line_width (cr, 2.0);
        cairo_stroke (cr);
      }
  }
  void
  enter_event() override
  {
    highlight = true;
    update();
  }
  void
  mouse_press (double x, double y) override
  {
    pressed = true;
    update();
  }
  void
  mouse_release (double x, double y) override
  {
    if (pressed && x >= 0 && y >= 0 && x < width && y < height)
      {
        signal_clicked();
      }

    pressed = false;
    update();
  }
  void
  leave_event() override
  {
    highlight = false;
    pressed = false;
    update();
  }
  void
  set_symbol (char s)
  {
    symbol = s;
    update();
  }
};

}

#endif
