// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LINEEDIT_HH
#define SPECTMORPH_LINEEDIT_HH

#include "smdrawutils.hh"
#include "smmath.hh"
#include "smwindow.hh"
#include "smtimer.hh"

namespace SpectMorph
{

class LineEdit : public Widget
{
protected:
  std::string m_text;
  bool highlight = false;
  bool click_to_focus = false;
  bool cursor_blink = false;
  int  cursor_pos = 3;
  std::vector<double> prefix_x;

public:
  void
  set_text (const std::string& new_text)
  {
    if (m_text == new_text)
      return;

    m_text = new_text;
    update();
  }
  std::string
  text() const
  {
    return m_text;
  }
  void
  set_click_to_focus (bool ctf)
  {
    click_to_focus = ctf;
  }
  LineEdit (Widget *parent, const std::string& start_text) :
    Widget (parent),
    m_text (start_text)
  {
    Timer *timer = new Timer (this);
    connect (timer->signal_timeout, this, &LineEdit::on_timer);
    timer->start (500);
  }
  void
  on_timer()
  {
    cursor_blink = !cursor_blink;
    update();
  }
  void
  draw (const DrawEvent& devent) override
  {
    DrawUtils du (devent.cr);

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

    /* compute prefix width array */
    std::vector<uint32> uc = utf8_to_unicode (m_text);
    prefix_x.clear();
    for (size_t i = 0; i < uc.size() + 1; i++)
      {
        std::vector<uint32> prefix = uc;
        prefix.resize (i);

        std::string b4 = utf8_from_unicode (prefix);
        double w_ = du.text_width ("_");
        double tw = du.text_width ("_" + b4 + "_") - 2 * w_; /* also count spaces at start/end */
        prefix_x.push_back (10 + tw);
      }
    /* draw cursor */
    if (window()->has_keyboard_focus (this) && cursor_blink)
      {
        du.rect_fill (prefix_x[cursor_pos], space * 2, 1, height - 4 * space, text_color);
      }
  }
  bool
  is_control (uint32 u)
  {
    return (u <= 0x1F) || (u >= 0x7F && u <= 0x9f);
  }
  static std::string
  utf8_from_unicode (const std::vector<uint32>& unicode)
  {
    std::string utf8 = "";
    for (auto c : unicode)
      {
        char buffer[8] = { 0, };
        g_unichar_to_utf8 (c, buffer);
        utf8 += buffer;
      }
    return utf8;
  }
  static std::vector<uint32>
  utf8_to_unicode (const std::string& utf8)
  {
    gunichar *uc = g_utf8_to_ucs4 (utf8.c_str(), -1, NULL, NULL, NULL);
    std::vector<uint32> chars;
    if (uc)
      {
        for (size_t i = 0; uc[i]; i++)
          chars.push_back (uc[i]);
      }
    g_free (uc);
    return chars;
  }
  virtual void
  key_press_event (const PuglEventKey& key_event) override
  {
    std::string old_text = m_text;

    if (key_event.filter)
      {
        /* multi key sequence -> ignore */
        return;
      }
    if (!is_control (key_event.character) && key_event.utf8[0])
      {
        std::vector<uint32> input = utf8_to_unicode ((const char *) key_event.utf8);
        std::vector<uint32> chars = utf8_to_unicode (m_text);
        chars.insert (chars.begin() + cursor_pos, input.begin(), input.end());
        m_text = utf8_from_unicode (chars);
        cursor_pos++;
      }
    else if ((key_event.character == PUGL_CHAR_BACKSPACE || key_event.character == PUGL_CHAR_DELETE) && !m_text.empty())
      {
        // Windows and Linux use backspace, macOS uses delete, so we support both (FIXME)
        std::vector<uint32> chars = utf8_to_unicode (m_text);

        if (key_event.character == PUGL_CHAR_BACKSPACE)
          {
            if (chars.size() && cursor_pos && cursor_pos <= int (chars.size()))
              chars.erase (chars.begin() + cursor_pos - 1);
            cursor_pos--;
          }
        else /* DELETE */
          {
            if (chars.size() && cursor_pos < int (chars.size()))
              chars.erase (chars.begin() + cursor_pos);
          }

        m_text = utf8_from_unicode (chars);
      }
    else if (key_event.character == 13)
      {
        signal_return_pressed();
      }
    else if (key_event.character == 27)
      {
        signal_esc_pressed();
      }
    else if (key_event.special == PUGL_KEY_LEFT)
      {
        cursor_pos = std::max (cursor_pos - 1, 0);
        update();
      }
    else if (key_event.special == PUGL_KEY_RIGHT)
      {
        std::vector<uint32> chars = utf8_to_unicode (m_text);
        cursor_pos = std::min<int> (cursor_pos + 1, chars.size());
        update();
      }
    if (m_text != old_text)
      {
        signal_text_changed (m_text);
        update();
      }
  }
  void
  enter_event() override
  {
    highlight = true;
    update();
  }
  void
  leave_event() override
  {
    highlight = false;
    update();
  }
  void
  focus_event() override
  {
    update();
  }
  void
  focus_out_event() override
  {
    signal_focus_out();
    update();
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    if (event.button == LEFT_BUTTON && click_to_focus)
      window()->set_keyboard_focus (this, true);
    if (event.button == LEFT_BUTTON && prefix_x.size())
      {
        double min_dist = 1e10;
        for (size_t i = 0; i < prefix_x.size(); i++)
          {
            double dist = fabs (prefix_x[i] - event.x);
            if (dist < min_dist)
              {
                cursor_pos = i;
                min_dist = dist;
              }
          }
        update();
      }
  }
  Signal<std::string> signal_text_changed;
  Signal<>            signal_return_pressed;
  Signal<>            signal_esc_pressed;
  Signal<>            signal_focus_out;
};

}

#endif

