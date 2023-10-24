// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smdrawutils.hh"
#include "smwindow.hh"

namespace SpectMorph
{

class VUMeter : public Widget
{
protected:
  double m_value = 0.0; /* 0.0 ... 1.0 */

public:
  void
  set_value (double new_value)
  {
    if (m_value == new_value)
      return;

    m_value = std::clamp (new_value, 0.0, 1.0);
    update();
  }
  double
  value() const
  {
    return m_value;
  }
  VUMeter (Widget *parent) :
    Widget (parent)
  {
  }
  void
  draw (const DrawEvent& devent) override
  {
    DrawUtils du (devent.cr);
    Color bg_color = Color (ThemeColor::WINDOW_BG).lighter();

    du.rect_fill (0, 0, width() * m_value, height(), ThemeColor::SLIDER);
    du.rect_fill (width() * m_value, 0, width(), height(), bg_color);
  }
};

}
