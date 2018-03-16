// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_LABEL_HH
#define SPECTMORPH_LABEL_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

struct Label : public Widget
{
  std::string text;
  TextAlign align = TextAlign::LEFT;
  bool bold = false;
  Color color = ThemeColor::TEXT;

  Label (Widget *parent, const std::string& text) :
    Widget (parent),
    text (text)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    Color text_color = color;

    if (!enabled())
      text_color = text_color.darker();

    du.set_color (text_color);
    du.bold = bold;
    du.text (text, 0, 0, width, height, align);
  }
};

}

#endif
