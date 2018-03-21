// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_LABEL_HH
#define SPECTMORPH_LABEL_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

class Label : public Widget
{
  std::string   m_text;
  TextAlign     m_align = TextAlign::LEFT;
  bool          m_bold = false;
  Color         m_color = ThemeColor::TEXT;

public:
  Label (Widget *parent, const std::string& text) :
    Widget (parent),
    m_text (text)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    Color text_color = m_color;

    if (!enabled())
      text_color = text_color.darker();

    du.set_color (text_color);
    du.bold = m_bold;
    du.text (m_text, 0, 0, width, height, m_align);
  }
  void
  set_bold (bool bold)
  {
    m_bold = bold;
  }
  void
  set_text (const std::string& text)
  {
    m_text = text;
  }
  void
  set_align (TextAlign align)
  {
    m_align = align;
  }
  void
  set_color (Color color)
  {
    m_color = color;
  }
};

}

#endif
