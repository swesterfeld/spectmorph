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
  std::u32string text32;
  bool highlight = false;
  bool click_to_focus = false;
  bool cursor_blink = false;
  int  cursor_pos = 0;
  std::vector<double> prefix_x;
public:
  void
  set_text (const std::string& new_text)
  {
    auto new_text32 = to_utf32 (new_text);
    if (text32 == new_text32)
      return;

    text32 = new_text32;
    cursor_pos = text32.size();
    update();
  }
  std::string
  text() const
  {
    return to_utf8 (text32);
  }
  void
  set_click_to_focus (bool ctf)
  {
    click_to_focus = ctf;
  }
  LineEdit (Widget *parent, const std::string& start_text) :
    Widget (parent)
  {
    text32 = to_utf32 (start_text);
    cursor_pos = text32.size();

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

    std::string text = to_utf8 (text32);
    du.set_color (text_color);
    du.text (text, 10, 0, width - 10, height);

    /* compute prefix width array */
    prefix_x.clear();
    for (size_t i = 0; i < text32.size() + 1; i++)
      {
        auto prefix = text32.substr (0, i);

        std::string b4 = to_utf8 (prefix);
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
  to_utf8 (const std::u32string& str)
  {
    std::string utf8;
    for (auto c : str)
      {
        char buffer[8] = { 0, };
        g_unichar_to_utf8 (c, buffer);
        utf8 += buffer;
      }
    return utf8;
  }
  static std::u32string
  to_utf32 (const std::string& utf8)
  {
    std::u32string utf32;
    gunichar *uc = g_utf8_to_ucs4 (utf8.c_str(), -1, NULL, NULL, NULL);
    if (uc)
      {
        for (size_t i = 0; uc[i]; i++)
          utf32.push_back (uc[i]);
      }
    g_free (uc);
    return utf32;
  }
  virtual void
  key_press_event (const PuglEventKey& key_event) override
  {
    std::u32string old_text32 = text32;

    if (key_event.filter)
      {
        /* multi key sequence -> ignore */
        return;
      }
    if (!is_control (key_event.character) && key_event.utf8[0])
      {
        std::u32string input = to_utf32 ((const char *) key_event.utf8);
        text32.insert (cursor_pos, input);
        cursor_pos++;
      }
    else if ((key_event.character == PUGL_CHAR_BACKSPACE || key_event.character == PUGL_CHAR_DELETE) && !text32.empty())
      {
        // Windows and Linux use backspace, macOS uses delete, so we support both (FIXME)

        if (key_event.character == PUGL_CHAR_BACKSPACE)
          {
            if (cursor_pos > 0 && cursor_pos <= int (text32.size()))
              text32.erase (cursor_pos - 1, 1);
            cursor_pos--;
          }
        else /* DELETE */
          {
            if (cursor_pos < int (text32.size()))
              text32.erase (cursor_pos, 1);
          }
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
        cursor_pos = std::min<int> (cursor_pos + 1, text32.size());
        update();
      }
    if (text32 != old_text32)
      {
        signal_text_changed (to_utf8 (text32));
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

