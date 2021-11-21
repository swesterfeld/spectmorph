// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
  draw (const DrawEvent& devent) override
  {
    DrawUtils du (devent.cr);

    Color text_color = m_color;

    if (!enabled())
      text_color = text_color.darker();

    du.set_color (text_color);
    du.bold = m_bold;
    du.text (m_text, 0, 0, width(), height(), m_align);
  }
  void
  set_bold (bool bold)
  {
    if (m_bold == bold)
      return;

    m_bold = bold;
    update();
  }
  void
  set_text (const std::string& text)
  {
    if (m_text == text)
      return;

    m_text = text;
    update();
  }
  void
  set_align (TextAlign align)
  {
    if (m_align == align)
      return;

    m_align = align;
    update();
  }
  void
  set_color (Color color)
  {
    if (m_color == color)
      return;

    m_color = color;
    update();
  }
};

}

#endif
