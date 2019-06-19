// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smlineedit.hh"

#include <locale>
#include <codecvt>

using namespace SpectMorph;

using std::string;
using std::u32string;
using std::min;
using std::max;

LineEdit::LineEdit (Widget *parent, const string& start_text) :
  Widget (parent)
{
  text32 = to_utf32 (start_text);
  cursor_pos = text32.size();

  Timer *timer = new Timer (this);
  connect (timer->signal_timeout, this, &LineEdit::on_timer);
  timer->start (500);
}

void
LineEdit::set_text (const string& new_text)
{
  auto new_text32 = to_utf32 (new_text);
  if (text32 == new_text32)
    return;

  text32 = new_text32;
  cursor_pos = text32.size();
  select_start = -1;
  update();
}

string
LineEdit::text() const
{
  return to_utf8 (text32);
}

void
LineEdit::set_click_to_focus (bool ctf)
{
  click_to_focus = ctf;
}

void
LineEdit::on_timer()
{
  cursor_blink = !cursor_blink;
  update();
}

void
LineEdit::draw (const DrawEvent& devent)
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

  /* compute prefix width array */
  prefix_x.clear();
  for (size_t i = 0; i < text32.size() + 1; i++)
    {
      auto prefix = text32.substr (0, i);

      string b4 = to_utf8 (prefix);
      double w_ = du.text_width ("_");
      double tw = du.text_width ("_" + b4 + "_") - 2 * w_; /* also count spaces at start/end */
      prefix_x.push_back (10 + tw + 1);
    }
  if (select_start >= 0 && select_start != cursor_pos)
    {
      const int select_l = prefix_x[min (select_start, cursor_pos)];
      const int select_r = prefix_x[max (select_start, cursor_pos)];

      du.rect_fill (select_l, space * 3, select_r - select_l, height - 6 * space, Color (0, 0.5, 0));
    }

  string text = to_utf8 (text32);
  du.set_color (text_color);
  du.text (text, 10, 0, width - 10, height);

  /* draw cursor */
  if (window()->has_keyboard_focus (this) && cursor_blink)
    du.rect_fill (prefix_x[cursor_pos] - 0.5, space * 3, 1, height - 6 * space, text_color);
}

bool
LineEdit::is_control (uint32 u)
{
  return (u <= 0x1F) || (u >= 0x7F && u <= 0x9f);
}

string
LineEdit::to_utf8 (const u32string& str)
{
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return conv.to_bytes (str);
}

u32string
LineEdit::to_utf32 (const string& utf8)
{
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return conv.from_bytes (utf8);
}

bool
LineEdit::overwrite_selection()
{
  if (select_start < 0)
    return false;

  int l = min (select_start, cursor_pos);
  int r = max (select_start, cursor_pos);
  text32.erase (l, r - l);
  cursor_pos = l;

  select_start = -1;
  return l != r;
}

void
LineEdit::select_all()
{
  select_start = 0;
  cursor_pos = text32.size();

  update();
}

void
LineEdit::key_press_event (const PuglEventKey& key_event)
{
  u32string old_text32 = text32;
  const bool mod_shift = key_event.state & PUGL_MOD_SHIFT;

  if (key_event.filter)
    {
      /* multi key sequence -> ignore */
      return;
    }
  if (key_event.character == 1) /* Ctrl+A */
    {
      select_all();
    }
  else if (!is_control (key_event.character) && key_event.utf8[0])
    {
      overwrite_selection();

      u32string input = to_utf32 ((const char *) key_event.utf8);
      text32.insert (cursor_pos, input);
      cursor_pos++;
    }
  else if ((key_event.character == PUGL_CHAR_BACKSPACE || key_event.character == PUGL_CHAR_DELETE) && !text32.empty())
    {
      // Windows and Linux use backspace, macOS uses delete, so we support both (FIXME)

      if (overwrite_selection())
        {
          // if there was a selection, we just overwrite it
        }
      else if (key_event.character == PUGL_CHAR_BACKSPACE)
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
      if (mod_shift && select_start == -1)
        select_start = cursor_pos;
      if (!mod_shift && select_start != -1)
        {
          cursor_pos = min (select_start, cursor_pos);
          select_start = -1;
        }
      else
        {
          cursor_pos = max (cursor_pos - 1, 0);
        }
      update();
    }
  else if (key_event.special == PUGL_KEY_RIGHT)
    {
      if (mod_shift && select_start == -1)
        select_start = cursor_pos;
      if (!mod_shift && select_start != -1)
        {
          cursor_pos = max (select_start, cursor_pos);
          select_start = -1;
        }
      else
        {
          cursor_pos = min<int> (cursor_pos + 1, text32.size());
        }
      update();
    }
  if (text32 != old_text32)
    {
      signal_text_changed (to_utf8 (text32));
      update();
    }
}

void
LineEdit::enter_event()
{
  highlight = true;
  update();
}

void
LineEdit::leave_event()
{
  highlight = false;
  update();
}

void
LineEdit::focus_event()
{
  update();
}

void
LineEdit::focus_out_event()
{
  signal_focus_out();
  select_start = -1; // clear selection
  update();
}

int
LineEdit::x_to_cursor_pos (double x)
{
  int pos = -1;
  double min_dist = 1e10;
  for (size_t i = 0; i < prefix_x.size(); i++)
    {
      double dist = fabs (prefix_x[i] - x);
      if (dist < min_dist)
        {
          pos = i;
          min_dist = dist;
        }
    }
  return pos;
}

bool
LineEdit::is_word_char (int pos)
{
  if (pos < 0 || pos >= int (text32.size()))
    return false;

  auto c = text32[pos];

  if (c >= 'A' && c <= 'Z')
    return true;
  if (c >= 'a' && c <= 'z')
    return true;
  if (c >= '0' && c <= '9')
    return true;
  if (c == '-' || c == '_')
    return true;
  return false;
}

void
LineEdit::mouse_press (const MouseEvent& event)
{
  const double time = get_time();
  const bool triple_click = (event.double_click &&
                             last_press_event.double_click &&
                             get_time() - last_press_time < 0.4);
  last_press_time = time;
  last_press_event = event;
  if (event.button == LEFT_BUTTON)
    {
      mouse_drag = false;
      if (click_to_focus)
        {
          window()->set_keyboard_focus (this, true);
          update();
        }
      if (triple_click)
        {
          select_all();
        }
      else if (event.double_click)
        {
          /* get position, but avoid past-last-character cursor pos */
          const int pos = min (x_to_cursor_pos (event.x), int (text32.size()) - 1);
          if (pos >= 0)
            {
              if (!is_word_char (pos))
                {
                  select_start = pos;
                  cursor_pos = pos + 1;
                }
              else
                {
                  select_start = pos;
                  while (is_word_char (select_start - 1))
                    select_start--;
                  cursor_pos = pos;
                  while (is_word_char (cursor_pos))
                    cursor_pos++;
                }
            }
          update();
        }
      else
        {
          const int pos = x_to_cursor_pos (event.x);
          if (pos >= 0)
            {
              select_start = pos;
              cursor_pos = pos;
              mouse_drag = true;
            }
          update();
        }
    }
}

void
LineEdit::mouse_move (const MouseEvent& event)
{
  if (mouse_drag) /* select by dragging */
    {
      const int pos = x_to_cursor_pos (event.x);
      if (pos >= 0)
        cursor_pos = pos;

      update();
    }
}

void
LineEdit::mouse_release (const MouseEvent& event)
{
  if (event.button == LEFT_BUTTON)
    mouse_drag = false;
}
