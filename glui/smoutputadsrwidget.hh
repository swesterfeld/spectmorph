// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_OUTPUT_ADSR_WIDGET_HH
#define SPECTMORPH_OUTPUT_ADSR_WIDGET_HH

#include "smwidget.hh"

namespace SpectMorph
{

class MorphOutputView;
class OutputADSRWidget : public Widget
{
  MorphOutput *morph_output;

  std::vector<Point> ps;
  int sel_point = -1;
  bool mouse_down = false;

  void
  draw_grid (cairo_t *cr)
  {
    DrawUtils du (cr);

    du.set_color (Color (0.33, 0.33, 0.33));
    cairo_set_line_width (cr, 1);

    const double pad = 8;
    for (double y = pad; y < height - 4; y += pad)
      {
        cairo_move_to (cr, 0, y);
        cairo_line_to (cr, width, y);
        cairo_stroke (cr);
      }
    for (double x = pad; x < width - 4; x += pad)
      {
        cairo_move_to (cr, x, 0);
        cairo_line_to (cr, x, height);
        cairo_stroke (cr);
      }
  }
public:
  OutputADSRWidget (Widget *parent, MorphOutput *morph_output, MorphOutputView *morph_output_view)
    : Widget (parent),
      morph_output (morph_output)
  {
  }

  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

    draw_grid (cr);
    /* redraw border to overdraw line endings */
    du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4), Color::null());

    Color line_color (ThemeColor::SLIDER);
    line_color = line_color.lighter();

    du.set_color (line_color);
    cairo_set_line_width (cr, 1);

    const double pad = 8;
    const double yspace = (width - 2 * pad) / 4;
    ps.clear();
    ps.push_back ({pad, height - pad});
    ps.push_back ({ps.back().x() + yspace * morph_output->adsr_attack() / 100, pad});
    ps.push_back ({ps.back().x() + yspace * morph_output->adsr_decay() / 100, pad + (height - 2 * pad) * (100 - morph_output->adsr_sustain()) / 100});
    ps.push_back ({ps.back().x() + yspace, ps.back().y()});
    ps.push_back ({ps.back().x() + yspace * morph_output->adsr_release() / 100, height - pad});

    for (size_t i = 0; i < ps.size(); i++)
      {
        if (i == 0)
          cairo_move_to (cr, ps[i].x(), ps[i].y());
        else
          cairo_line_to (cr, ps[i].x(), ps[i].y());
      }
    cairo_stroke (cr);

    if (sel_point > 0)
      {
        du.set_color (Color (0.5, 0.5, 0.5));

        cairo_move_to (cr, ps[sel_point].x(), 0);
        cairo_line_to (cr, ps[sel_point].x(), height);
        cairo_stroke (cr);
      }
    for (size_t i = 1; i < ps.size(); i++)
      {
        Color c_color (0.8, 0.8, 0.8);
        double R = 4;
        if (sel_point == int (i))
          {
            R += 2;
            c_color = ThemeColor::SLIDER;
            if (mouse_down) // indicate drag
              c_color = c_color.lighter();
            else
              c_color = Color (1.0, 1.0, 1.0);
          }
        du.round_box (ps[i].x() - R, ps[i].y() - R, 2 * R, 2 * R, 1, R, c_color, c_color);
      }
  }

  void
  motion (double x, double y) override
  {
    if (!mouse_down)
      {
        double min_dist = 1e8;

        for (size_t i = 1; i < ps.size(); i++)
          {
            // this ensures that the x-values of the points are a little bit different
            // (allows selecting one of two points with same x value)
            const double small_epsilon = 1e-5 * i;
            const double dist = fabs (x - (ps[i].x() + small_epsilon));

            if (dist < min_dist)
              {
                sel_point = i;
                min_dist = dist;
                update();
              }
          }
      }
    else
      {
        // drag
        const double pad = 8;
        const double yspace = (width - 2 * pad) / 4;

        if (sel_point > 0)
          {
            double new_x_percent = sm_bound (0.0, 100.0 * (x - ps[sel_point - 1].x()) / yspace, 100.0);
            double new_y_percent = sm_bound (0.0, 100.0 * (1.0 - (y - pad) / yspace), 100.0);

            if (sel_point == 1) // A
              morph_output->set_adsr_attack (new_x_percent);

            if (sel_point == 2) // D
              {
                morph_output->set_adsr_decay (new_x_percent);
                morph_output->set_adsr_sustain (new_y_percent);
              }

            if (sel_point == 3) // S
                morph_output->set_adsr_sustain (new_y_percent);

            if (sel_point == 4) // R
              morph_output->set_adsr_release (new_x_percent);

            signal_adsr_params_changed();

            update();
          }
      }
  }

  void
  mouse_press (double x, double y) override
  {
    mouse_down = true;
    update();
  }
  void
  mouse_release (double x, double y) override
  {
    mouse_down = false;
    update();
  }

  void
  leave_event() override
  {
    sel_point = -1;
    update();
  }

/* slots: */
  void
  on_adsr_params_changed()
  {
    update();
  }

/* signals: */
  Signal<> signal_adsr_params_changed;
};

}

#endif

