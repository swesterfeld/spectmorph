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
  bool mouse_drag = false;
  bool cursor_blink = false;
  int  cursor_pos = 0;
  int  select_start = -1;
  std::vector<double> prefix_x;
  MouseEvent last_press_event; /* triple click detection */
  double last_press_time = 0;

  bool is_control (uint32 u);
  int  x_to_cursor_pos (double x);
  bool is_word_char (int pos);
  bool overwrite_selection();

  static std::string to_utf8 (const std::u32string& str);
  static std::u32string to_utf32 (const std::string& utf8);
public:
  LineEdit (Widget *parent, const std::string& start_text);

  void set_text (const std::string& new_text);
  std::string text() const;
  void select_all();
  void set_click_to_focus (bool ctf);

  void draw (const DrawEvent& devent) override;
  void key_press_event (const PuglEventKey& key_event) override;
  void enter_event() override;
  void leave_event() override;
  void focus_event() override;
  void focus_out_event() override;
  void mouse_press (const MouseEvent& event) override;
  void mouse_move (const MouseEvent& event) override;
  void mouse_release (const MouseEvent& event) override;
  void on_timer();

  Signal<std::string> signal_text_changed;
  Signal<>            signal_return_pressed;
  Signal<>            signal_esc_pressed;
  Signal<>            signal_focus_out;
};

}

#endif

