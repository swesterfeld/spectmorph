// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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

public:
  Signal<double> signal_value_changed;
  Signal<int>    signal_int_value_changed;

  Slider (Widget *parent, double value) :
    Widget (parent),
    m_value (value)
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
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double H = 6; // height of slider thing
    double C = 6;
    double value_pos = C + (width - C * 2) * m_value;

    Color slider_color_l = ThemeColor::SLIDER;
    if (enabled())
      {
        if (highlight)
          slider_color_l = slider_color_l.lighter();
      }
    else
      slider_color_l.set_rgb (0.4, 0.4, 0.4);
    du.round_box (0, height / 2 - H / 2, value_pos, H, 1, 2, slider_color_l.darker(), slider_color_l);

    Color slider_color_r (0.3, 0.3, 0.3);
    if (highlight)
      slider_color_r = slider_color_r.lighter();
    du.round_box (value_pos, height / 2 - H / 2, (width - value_pos), H, 1, 2, slider_color_r.darker(), slider_color_r);

    if (enabled())
      {
        if (highlight || mouse_down)
          cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
        else
          cairo_set_source_rgb (cr, 0.8, 0.8, 0.8);
      }
    else
      {
        cairo_set_source_rgb (cr, 0.4, 0.4, 0.4);
      }
    cairo_arc (cr, value_pos, height / 2, C, 0, 2 * M_PI);
    cairo_fill (cr);
  }
  void
  slider_value_from_x (double x)
  {
    double C = 6;
    m_value = sm_bound (0.0, (x - C) / (width - C * 2), 1.0);

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
  motion (double x, double y) override
  {
    if (mouse_down)
      slider_value_from_x (x);
  }
  void
  mouse_press (double x, double y) override
  {
    slider_value_from_x (x);
    mouse_down = true;
  }
  void
  mouse_release (double x, double y) override
  {
    mouse_down = false;
  }
  void
  enter_event() override
  {
    highlight = true;
  }
  void
  leave_event() override
  {
    highlight = false;
  }
  void
  set_value (double v)
  {
    m_value = v;
  }
  double
  value() const
  {
    return m_value;
  }
};

}

#endif
