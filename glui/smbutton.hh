// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_BUTTON_HH
#define SPECTMORPH_BUTTON_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

struct Button : public Widget
{
  std::string text;
  bool        highlight = false;
  bool        pressed = false;

  Signal<> signal_clicked;

  Button (Widget *parent, const std::string& text) :
    Widget (parent),
    text (text)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    if (enabled())
      cairo_set_source_rgba (cr, 1, 1, 1, 1);
    else
      cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 1);

    double space = 2;
    if (highlight)
      cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
    else
      cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    if (pressed)
      cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);

    du.round_box (space, space, width - 2 * space, height - 2 * space, 1, 10, true);

    cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 1);
    if (pressed)
      cairo_set_source_rgb (cr, 0.4, 0.4, 0.4);

    du.round_box (space, space, width - 2 * space, height - 2 * space, 1, 10);

    cairo_set_source_rgba (cr, 1, 1, 1, 1);
    du.text (text, 0, 0, width, height, TextAlign::CENTER);
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
        signal_clicked();
      }
    pressed = false;
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
