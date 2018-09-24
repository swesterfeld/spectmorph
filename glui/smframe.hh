// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_FRAME_HH
#define SPECTMORPH_FRAME_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

class Frame : public Widget
{
protected:
  Color m_frame_color = ThemeColor::FRAME;

public:
  Frame (Widget *parent)
    : Widget (parent)
  {
  }
  void
  draw (const DrawEvent& devent) override
  {
    const double radius  = 10;
    const double line_width = 1.5;

    DrawUtils du (devent.cr);
    du.round_box (0, 0, width, height, line_width, radius, m_frame_color, ThemeColor::OPERATOR_BG);
  }
  void
  set_frame_color (Color color)
  {
    if (color == m_frame_color)
      return;

    m_frame_color = color;
    update();
  }
};

}

#endif

