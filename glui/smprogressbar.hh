// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROGRESSBAR_HH
#define SPECTMORPH_PROGRESSBAR_HH

#include "smdrawutils.hh"
#include "smmath.hh"
#include "smwindow.hh"
#include "smeventloop.hh"

namespace SpectMorph
{

struct ProgressBar : public Widget
{
protected:
  double m_value = 0.0; /* 0.0 ... 1.0 */
  double busy_pos = 0;
  double last_time = 0;

public:
  void
  set_value (double new_value)
  {
    if (m_value == new_value)
      return;

    if (new_value < -0.5)
      m_value = -1;
    else
      m_value = sm_bound (0.0, new_value, 1.0);
    update();
  }
  double
  value() const
  {
    return m_value;
  }
  ProgressBar (Widget *parent) :
    Widget (parent)
  {
    connect (window()->event_loop()->signal_before_process, this, &ProgressBar::on_update_busy);
  }
  void
  draw (const DrawEvent& devent) override
  {
    DrawUtils du (devent.cr);

    double space = 2;

    Color frame_color = ThemeColor::FRAME;
    if (!recursive_enabled())
      frame_color = frame_color.darker();

    du.round_box (0, space, width, height - 2 * space, 1, 5, frame_color);

    auto draw_box = [&] (double x, double w) {
      double radius = 5;
      if (radius > w / 2)
        radius = w / 2;
      du.round_box (x, space * 2, w, height - 4 * space, 0.0, radius, Color::null(), ThemeColor::SLIDER);
    };
    if (m_value < -0.5)
      {
        double x = space + busy_pos * (width - 2 * space);
        if (busy_pos > 0.75)
          {
            draw_box (x, (width - 2 * space) * (1 - busy_pos));
            draw_box (space, (width - 2 * space) * (busy_pos - 0.75));
          }
        else
          draw_box (x, (width - 2 * space) * 0.25);
      }
    else
      {
        draw_box (space, (width - 2 * space) * m_value);
      }
  }
  void
  on_update_busy()
  {
    if (m_value < 0)
      {
        const double time = get_time();
        const double step = (time - last_time) * 0.4;
        last_time = time;

        if (step < 1)
          busy_pos += step;
        if (busy_pos > 1)
          busy_pos -= 1;
        update();
      }
  }
};

}

#endif
