// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_FRAME_HH
#define SPECTMORPH_FRAME_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

struct Frame : public Widget
{
  Frame (Widget *parent)
    : Widget (parent)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    const double radius  = 10;
    const double line_width = 1.5;

    DrawUtils du (cr);
    du.round_box (0, 0, width, height, line_width, radius, ThemeColor::FRAME, ThemeColor::OPERATOR_BG);
}
};

}

#endif

