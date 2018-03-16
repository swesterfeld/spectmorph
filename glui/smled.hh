// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LED_HH
#define SPECTMORPH_LED_HH

namespace SpectMorph
{

class Led : public Widget
{
  bool m_on;
public:

  Led (Widget *parent, bool on) :
    Widget (parent),
    m_on (on)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);
    double radius = std::min (width, height) / 2 - 2;

    Color lcolor = m_on ? Color (0, 1, 0) : Color (0, 0.5, 0);

    // circle
    cairo_arc (cr, width / 2, height / 2, radius, 0, 2 * M_PI);
    du.set_color (lcolor);
    cairo_fill_preserve (cr);

    // frame
    cairo_set_line_width (cr, 1);
    du.set_color (lcolor.darker());
    cairo_stroke (cr);
  }
  void
  set_on (bool on)
  {
    m_on = on;
    update();
  }
};

}

#endif
