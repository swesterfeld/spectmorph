// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_BUTTON_HH
#define SPECTMORPH_BUTTON_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

class Button : public Widget
{
  std::string m_text;
  bool        highlight = false;
  bool        pressed = false;

public:
  Signal<> signal_clicked;
  Signal<> signal_pressed;
  Signal<> signal_released;

  Button (Widget *parent, const std::string& text) :
    Widget (parent),
    m_text (text)
  {
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    double space = 2;
    Color bg_color;
    if (highlight)
      bg_color.set_rgb (0.7, 0.7, 0.7);
    else
      bg_color.set_rgb (0.5, 0.5, 0.5);
    if (pressed)
      bg_color.set_rgb (0.3, 0.3, 0.3);

    Color frame_color (0.3, 0.3, 0.3);
    if (pressed)
      frame_color.set_rgb (0.4, 0.4, 0.4);

    if (!recursive_enabled())
      bg_color.set_rgb (0.3, 0.3, 0.3);

    du.round_box (space, space, width - 2 * space, height - 2 * space, 1, 10, frame_color, bg_color);

    Color text_color (1, 1, 1);
    if (!recursive_enabled())
      text_color = Color (0.7, 0.7, 0.7);

    du.set_color (text_color);
    du.text (m_text, 0, 0, width, height, TextAlign::CENTER);
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
    signal_pressed();
  }
  void
  mouse_release (double x, double y) override
  {
    if (!pressed)
      return;

    pressed = false;
    update();
    signal_released();

    if (x >= 0 && y >= 0 && x < width && y < height)
      signal_clicked();  // this must be the last line, as deletion can occur afterwards
  }
  void
  leave_event() override
  {
    highlight = false;
    pressed = false;
    update();
  }
  void
  set_text (const std::string& text)
  {
    if (m_text == text)
      return;

    m_text = text;
    update();
  }
};

}

#endif
