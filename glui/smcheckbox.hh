// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_CHECKBOX_HH
#define SPECTMORPH_CHECKBOX_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

class CheckBox : public Widget
{
  std::string text;
  bool highlight = false;
  bool checked = false;
  double end_x = 0;

public:
  Signal<bool> signal_toggled;

  CheckBox (Widget *parent, const std::string& text) :
    Widget (parent),
    text (text)
  {
  }
  void
  set_checked (bool new_checked)
  {
    if (checked != new_checked)
      {
        checked = new_checked;
        update();
      }
  }
  void
  set_text (const std::string& new_text)
  {
    if (text == new_text)
      return;

    text = new_text;
    update();
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    double space = 2;

    Color frame_color, fill_color;

    if (checked)
      {
        fill_color = ThemeColor::CHECKBOX;
      }
    else
      {
        fill_color.set_rgb (0.5, 0.5, 0.5);
      }
    if (highlight)
      fill_color = fill_color.lighter();

    frame_color = fill_color.darker();

    du.round_box (0, space, 16 - 2 * space, height - 2 * space, 1, 2, frame_color, fill_color);

    if (enabled())
      cairo_set_source_rgba (cr, 1, 1, 1, 1);
    else
      cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 1);

    du.text (text, 16, 0, width - 16, height);
    end_x = du.text_width (text) + 16;
  }
  void
  mouse_move (const MouseEvent& event) override
  {
    bool new_highlight = event.x < end_x;

    if (new_highlight != highlight)
      {
        highlight = new_highlight;
        update();
      }
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    if (event.button == LEFT_BUTTON && highlight)
      {
        checked = !checked;
        signal_toggled (checked);
        update();
      }
  }
  void
  leave_event() override
  {
    highlight = false;
    update();
  }
};

}

#endif
