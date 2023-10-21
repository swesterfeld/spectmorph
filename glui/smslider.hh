// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_SLIDER_HH
#define SPECTMORPH_SLIDER_HH

#include <math.h>
#include "smmath.hh"

namespace SpectMorph
{

class Slider : public Widget
{
  double m_value;
  bool highlight = false;
  bool mouse_down = false;
  int int_range_min = 0;
  int int_range_max = 0;
  bool shift_drag = false;
  double shift_drag_start_value = 0;
  MouseEvent shift_drag_start_event;
  const Orientation orientation;

public:
  Signal<double> signal_value_changed;
  Signal<int>    signal_int_value_changed;

  Slider (Widget *parent, double value, Orientation orientation = Orientation::HORIZONTAL) :
    Widget (parent),
    m_value (value),
    orientation (orientation)
  {
  }
  void
  set_int_range (int mn, int mx)
  {
    int_range_min = mn;
    int_range_max = mx;
  }
  void
  set_int_value (int ivalue)
  {
    m_value = double (ivalue - int_range_min) / (int_range_max - int_range_min);
    update();
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    double SIZE = 6; // height or width of the slider thing
    double C = 6;

    Color slider_color_l = ThemeColor::SLIDER;
    if (enabled())
      {
        if (highlight)
          slider_color_l = slider_color_l.lighter();
      }
    else
      slider_color_l.set_rgb (0.4, 0.4, 0.4);

    Color slider_color_r (0.3, 0.3, 0.3);
    if (highlight)
      slider_color_r = slider_color_r.lighter();

    Color circle_color;
    if (enabled())
      {
        if (highlight || mouse_down)
          circle_color.set_rgb (1.0, 1.0, 1.0);
        else
          circle_color.set_rgb (0.8, 0.8, 0.8);
      }
    else
      {
        circle_color.set_rgb (0.4, 0.4, 0.4);
      }

    if (orientation == Orientation::HORIZONTAL)
      {
        double value_pos = C + (width() - C * 2) * m_value;

        du.round_box (0, height() / 2 - SIZE / 2,         value_pos, SIZE,             1, 2, slider_color_l.darker(), slider_color_l);
        du.round_box (value_pos, height() / 2 - SIZE / 2, (width() - value_pos), SIZE, 1, 2, slider_color_r.darker(), slider_color_r);
        du.circle    (value_pos, height() / 2, C, circle_color);
      }
    else
      {
        double value_pos = C + (height() - C * 2) * (1 - m_value);

        du.round_box (width() / 2 - SIZE / 2, 0,         SIZE, value_pos,              1, 2, slider_color_l.darker(), slider_color_l);
        du.round_box (width() / 2 - SIZE / 2, value_pos, SIZE, (height() - value_pos), 1, 2, slider_color_r.darker(), slider_color_r);
        du.circle    (width() / 2, value_pos, C, circle_color);
      }
  }
  void
  slider_value_from_event (const MouseEvent& event)
  {
    constexpr double shift_drag_speed = 8;
    constexpr double C = 6;

    if (orientation == Orientation::HORIZONTAL)
      {
        if (shift_drag)
          m_value = shift_drag_start_value + (event.x - shift_drag_start_event.x) / (width() - C * 2) / shift_drag_speed;
        else
          m_value = (event.x - C) / (width() - C * 2);
      }
    else
      {
        if (shift_drag)
          m_value = shift_drag_start_value - (event.y - shift_drag_start_event.y) / (height() - C * 2) / shift_drag_speed;
        else
          m_value = 1 - (event.y - C) / (height() - C * 2);
      }
    m_value = std::clamp (m_value, 0.0, 1.0);

    /* optional: only allow discrete integer values */
    if (int_range_min != int_range_max)
      {
        int ivalue = int_range_min + sm_round_positive (m_value * (int_range_max - int_range_min));
        m_value = double (ivalue - int_range_min) / (int_range_max - int_range_min);

        signal_int_value_changed (ivalue);
      }

    signal_value_changed (m_value);
  }
  void
  mouse_move (const MouseEvent& event) override
  {
    if (mouse_down)
      {
        slider_value_from_event (event);
        update();
      }
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    if (event.button == LEFT_BUTTON)
      {
        shift_drag = (event.state & PUGL_MOD_SHIFT);
        if (shift_drag)
          {
            shift_drag_start_event = event;
            shift_drag_start_value = m_value;
          }
        else
          {
            slider_value_from_event (event);
          }
        mouse_down = true;
        update();
      }
  }
  void
  mouse_release (const MouseEvent& event) override
  {
    if (event.button == LEFT_BUTTON)
      {
        mouse_down = false;
        update();
      }
  }
  void
  enter_event() override
  {
    highlight = true;
    update();
  }
  void
  leave_event() override
  {
    highlight = false;
    update();
  }
  void
  set_value (double v)
  {
    m_value = v;
    update();
  }
  double
  value() const
  {
    return m_value;
  }
};

}

#endif
