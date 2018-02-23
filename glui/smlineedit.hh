// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LINEEDIT_HH
#define SPECTMORPH_LINEEDIT_HH

#include "smdrawutils.hh"
#include "smmath.hh"

namespace SpectMorph
{

struct LineEdit : public Widget
{
protected:
  std::string m_text;
  bool highlight = false;

public:
  void
  set_text (const std::string& new_text)
  {
    m_text = new_text;
  }
  std::string
  text() const
  {
    return m_text;
  }
  LineEdit (Widget *parent)
    : Widget (parent)
  {
    m_text = "/tmp/foo.smplan";
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;
    Color fill_color;
    if (highlight)
      fill_color = ThemeColor::MENU_BG;

    Color text_color (1, 1, 1);
    Color frame_color = ThemeColor::FRAME;
    if (!recursive_enabled())
      {
        text_color = text_color.darker();
        frame_color = frame_color.darker();
      }

    du.round_box (0, space, width, height - 2 * space, 1, 5, frame_color, fill_color);

    du.set_color (text_color);
    du.text (m_text, 10, 0, width - 10, height);
    double tw = du.text_width (m_text);
    du.round_box (10 + tw + 1, space * 2, 10, height - 4 * space, 1, 0, Color::null(), ThemeColor::SLIDER /* FIXME */ );
  }
  virtual void
  key_press_event (const PuglEventKey& key_event)
  {
    if (isprint (key_event.character))
      m_text += key_event.character;
    else if (key_event.character == PUGL_CHAR_BACKSPACE && !m_text.empty())
      m_text = m_text.substr (0, m_text.size() - 1);
  }
  void
  enter_event() override
  {
    highlight = true;
  }
  void
  leave_event() override
  {
    highlight = false;
  }
};

}

#endif

