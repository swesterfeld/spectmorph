// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smvoicestatus.hh"

namespace SpectMorph
{

class ControlStatus : public Widget
{
  std::vector<float> voices;
  Property& property;

  double
  value_pos (double v)
  {
    const int BORDER = 4;
    double spr_w, spr_h;
    window()->get_sprite_size (spr_w, spr_h);

    return spr_w / 2 + BORDER + (width() - spr_w - 2 * BORDER) * (v + 1) / 2;
  }
  void
  redraw_voices()
  {
    double spr_w, spr_h;
    window()->get_sprite_size (spr_w, spr_h);

    for (auto v : voices)
      update (value_pos (v) - spr_w / 2, height() / 2 - spr_h / 2, spr_w, spr_h, UPDATE_LOCAL);
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

    double spr_w, spr_h;
    window()->get_sprite_size (spr_w, spr_h);

    for (auto v : voices)
      window()->draw_sprite (this, value_pos (v) - spr_w / 2, height() / 2 - spr_h / 2);
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
