// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smvoicestatus.hh"

namespace SpectMorph
{

class ControlStatus : public Widget
{
  std::vector<float> voices;
  Property& property;

  static constexpr double RADIUS = 6;
  double
  value_pos (double v)
  {
    double X = 8;
    return RADIUS + X + (width() - (RADIUS + X) * 2) * (v + 1) / 2;
  }
  void
  redraw_voices()
  {
    for (auto v : voices)
      update (value_pos (v) - RADIUS - 1, height() / 2 - RADIUS - 1, RADIUS * 2 + 2, RADIUS * 2 + 2, UPDATE_LOCAL);
  }
public:
  ControlStatus (Widget *parent, Property& property) :
    Widget (parent),
    property (property)
  {
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
        cairo_arc (cr, value_pos (v), height() / 2 , RADIUS, 0, 2 * M_PI);
        du.set_color (vcolor);
        cairo_fill (cr);
      }
  }
  void
  on_voice_status_changed (VoiceStatus *voice_status)
  {
    redraw_voices();
    voices = voice_status->get_values (property);
    redraw_voices();
  }
};

}
