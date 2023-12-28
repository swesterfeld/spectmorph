// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smvoicestatus.hh"

namespace SpectMorph
{

class ControlStatus : public Widget
{
  Property& property;

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
    draw_sprites();
  }
  void
  on_voice_status_changed (VoiceStatus *voice_status)
  {
    auto [spr_w, spr_h] = window()->get_sprite_size();
    const int border = 4 + spr_w / 2;

    clear_sprites();
    for (auto v : voice_status->get_values (property))
      {
        Point p (border + (width() - 2 * border) * (v + 1) / 2, height() / 2);
        add_sprite (p);
      }
  }
};

}
