// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SLIDER_HH
#define SPECTMORPH_SLIDER_HH

namespace SpectMorph
{

struct Slider : public Widget
{
  double value;
  bool highlight = false;
  bool mouse_down = false;
  bool enter = false;
  std::function<void(float)> m_callback;

  Slider (Widget *parent, double x, double y, double width, double height, double value)
    : Widget (parent, x, y, width, height),
      value (value)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    /*if (enter)
      debug_fill (cr); */

    double H = 8; // height of slider thing
    double C = 12;
    double value_pos = C + (width - C * 2) * value;

    cairo_rectangle (cr, C, height / 2 - H / 2, value_pos, H);
    cairo_set_source_rgb (cr, 0.1, 0.9, 0.1);
    cairo_fill (cr);

    cairo_rectangle (cr, value_pos, height / 2 - H / 2, (width - C - value_pos), H);
    cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
    cairo_fill (cr);

    if (highlight || mouse_down)
      cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    else
      cairo_set_source_rgb (cr, 0.8, 0.8, 0.8);
    cairo_arc (cr, value_pos, height / 2, C, 0, 2 * M_PI);
    cairo_fill (cr);
  }
  bool
  in_circle (double x, double y)
  {
    double C = 12;
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
        double C = 12;
        value = (x - C) / (width - C * 2);
        if (value < 0)
          value = 0;
        if (value > 1)
          value = 1;
        if (m_callback)
          m_callback (value);
      }
  }
  void
  set_callback (const std::function<void(float)> &callback)
  {
    m_callback = callback;
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

