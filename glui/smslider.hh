// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SLIDER_HH
#define SPECTMORPH_SLIDER_HH

#include <math.h>
#include "smmath.hh"

namespace SpectMorph
{

struct Slider : public Widget
{
  double value;
  bool highlight = false;
  bool mouse_down = false;
  bool enter = false;
  int int_range_min = 0;
  int int_range_max = 0;
  Signal<double> signal_value_changed;
  Signal<int>    signal_int_value_changed;

  Slider (Widget *parent, double value) :
    Widget (parent),
    value (value)
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
    value = double (ivalue - int_range_min) / (int_range_max - int_range_min);
  }
  void
  draw (cairo_t *cr) override
  {
    double H = 4; // height of slider thing
    double C = 6;
    double value_pos = C + (width - C * 2) * value;

    cairo_rectangle (cr, C, height / 2 - H / 2, value_pos, H);
    if (enabled())
      cairo_set_source_rgb (cr, 0.1, 0.9, 0.1);
    else
      cairo_set_source_rgb (cr, 0.4, 0.4, 0.4);
    cairo_fill (cr);

    cairo_rectangle (cr, value_pos, height / 2 - H / 2, (width - C - value_pos), H);
    cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
    cairo_fill (cr);

    if (enabled())
      {
        if (highlight || mouse_down)
          cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
        else
          cairo_set_source_rgb (cr, 0.8, 0.8, 0.8);
      }
    else
      {
        cairo_set_source_rgb (cr, 0.6, 0.6, 0.6);
      }
    cairo_arc (cr, value_pos, height / 2, C, 0, 2 * M_PI);
    cairo_fill (cr);
  }
  bool
  in_circle (double x, double y)
  {
    double C = 6;
    double value_pos = C + (width - C * 2) * value;

    double dx = value_pos - x;
    double dy = height / 2 - y;
    double dist = sqrt (dx * dx + dy * dy);

    return (dist < C);
  }
  void
  motion (double x, double y) override
  {
    highlight = in_circle (x, y);
    if (mouse_down)
      {
        double C = 6;
        value = sm_bound (0.0, (x - C) / (width - C * 2), 1.0);

        /* optional: only allow discrete integer values */
        if (int_range_min != int_range_max)
          {
            int ivalue = int_range_min + sm_round_positive (value * (int_range_max - int_range_min));
            value = double (ivalue - int_range_min) / (int_range_max - int_range_min);

            signal_int_value_changed (ivalue);
          }

        signal_value_changed (value);
      }
  }
  void
  mouse_press (double x, double y) override
  {
    if (in_circle (x, y))
      {
        mouse_down = true;
      }
  }
  void
  mouse_release (double x, double y) override
  {
    mouse_down = false;
  }
  void
  enter_event() override
  {
    enter = true;
  }
  void
  leave_event() override
  {
    enter = false;
    highlight = false;
  }
};

}

#endif

