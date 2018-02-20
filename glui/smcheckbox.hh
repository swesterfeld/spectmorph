// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_CHECKBOX_HH
#define SPECTMORPH_CHECKBOX_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

struct CheckBox : public Widget
{
  std::string text;
  bool highlight = false;
  bool checked = false;
  double end_x = 0;

  Signal<bool> signal_toggled;

  CheckBox (Widget *parent, const std::string& text) :
    Widget (parent),
    text (text)
  {
  }
  void
  set_checked (bool new_checked)
  {
    checked = new_checked;
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;

    Color frame_color, fill_color;

    if (checked)
      {
        frame_color.set_rgb (0.1, 0.5, 0.1);

        if (highlight)
          fill_color.set_rgb (0.1, 0.9, 0.1);
        else
          fill_color.set_rgb (0.1, 0.7, 0.1);
      }
    else
      {
        frame_color.set_rgb (0.3, 0.3, 0.3);

        if (highlight)
          fill_color.set_rgb (0.7, 0.7, 0.7);
        else
          fill_color.set_rgb (0.5, 0.5, 0.5);
      }

    du.round_box (0, space, 16 - 2 * space, height - 2 * space, 1, 2, frame_color, fill_color);

    if (enabled())
      cairo_set_source_rgba (cr, 1, 1, 1, 1);
    else
      cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 1);

    du.text (text, 16, 0, width - 16, height);
    end_x = du.text_width (text) + 16;
  }
  void
  motion (double x, double y) override
  {
    highlight = x < end_x;
  }
  void
  mouse_press (double x, double y) override
  {
    if (highlight)
      {
        checked = !checked;
        signal_toggled (checked);
      }
  }
  void
  leave_event() override
  {
    highlight = false;
  }
};

}

#endif
