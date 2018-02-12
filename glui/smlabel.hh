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

  Label (Widget *parent, const std::string& text) :
    Widget (parent),
    text (text)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    if (enabled())
      cairo_set_source_rgba (cr, 1, 1, 1, 1);
    else
      cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 1);

    du.bold = bold;
    du.text (text, 0, 0, width, height, align);
  }
};

}

#endif
