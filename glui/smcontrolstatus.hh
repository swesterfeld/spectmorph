// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_CONTROL_STATUS_HH
#define SPECTMORPH_CONTROL_STATUS_HH

namespace SpectMorph
{

class ControlStatus : public Widget
{
  std::vector<float> voices;
public:
  ControlStatus (Widget *parent) :
    Widget (parent)
  {
  }
  void
  reset_voices()
  {
    voices.clear();
    update();
  }
  void
  add_voice (float value)
  {
    voices.push_back (value);
    update();
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);
    double space = 2;
    du.round_box (0, space, width(), height() - 2 * space, 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

    Color vcolor = Color (0.7, 0.7, 0.7);

    // circle
    for (auto v : voices)
      {
        double C = 6;
        double X = 8;
        double value_pos = C + X + (width() - (C + X) * 2) * (v + 1) / 2;

        cairo_arc (cr, value_pos, height() / 2 , C, 0, 2 * M_PI);
        du.set_color (vcolor);
        cairo_fill_preserve (cr);
      }
  }
};

}

#endif

