// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmessagebox.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smbutton.hh"
#include "smsimplelines.hh"

using std::vector;
using std::string;
using std::u32string;
using std::max;

using namespace SpectMorph;

static void
split_line (Window *window, vector<string>& lines, const string& line, double max_label_width)
{
  u32string part32;
  u32string line32 = to_utf32 (line);
  for (auto c32 : line32)
    {
      if (DrawUtils::static_text_extents (window, to_utf8 (part32 + c32)).x_advance > max_label_width)
        {
          int word_boundary = -1;
          for (size_t p = 0; p < part32.size(); p++)
            {
              if (part32[p] == ' ')
                word_boundary = p;
            }
          if (word_boundary > 0) // word boundary splitting
            {
              lines.push_back (to_utf8 (part32.substr (0, word_boundary)));
              part32 = part32.substr (word_boundary + 1);
            }
          else // char splitting
            {
              lines.push_back (to_utf8 (part32));
              part32.clear();
            }
        }

      part32 += c32;
    }
  if (!part32.empty())
    lines.push_back (to_utf8 (part32));
}

static vector<string>
split (Window *window, const string& text, double max_label_width)
{
  vector<string> lines;

  string s;
  for (char c : text)
    {
      if (c == '\n')
        {
          split_line (window, lines, s, max_label_width);
          s = "";
        }
      else
        {
          s += c;
        }
    }
  if (s != "")
    split_line (window, lines, s, max_label_width);
  return lines;
}


MessageBox::MessageBox (Window *window, const string& title, const string& text, Buttons buttons) :
  Dialog (window)
{
  FixedGrid grid;

  /* create buttons first to get width */
  vector<Button *> bwidgets;
  if (buttons & Buttons::OK)
    {
      auto button = new Button (this, "Ok");
      bwidgets.push_back (button);
      connect (button->signal_clicked, this, &Dialog::on_accept);
    }

  if (buttons & Buttons::CANCEL)
    {
      auto button = new Button (this, "Cancel");
      bwidgets.push_back (button);
      connect (button->signal_clicked, this, &Dialog::on_reject);
    }

  if (buttons & Buttons::SAVE)
    {
      auto button = new Button (this, "Save");
      bwidgets.push_back (button);
      connect (button->signal_clicked, this, &Dialog::on_accept);
    }

  if (buttons & Buttons::REVERT)
    {
      auto button = new Button (this, "Revert");
      bwidgets.push_back (button);
      connect (button->signal_clicked, this, &Dialog::on_reject);
    }
  const double button_width = 10 + (bwidgets.size() - 1) * 11;
  const double xframe = 2;
  const double max_label_width = window->width - 2 * xframe * 8 - 16;

  // window width
  double w = max (20.0, button_width);
  vector<string> lines = split (window, text, max_label_width);
  for (const auto& line : lines)
    w = max (w, DrawUtils::static_text_extents (window, line).x_advance / 8);

  auto title_label = new Label (this, title);
  title_label->set_bold (true);
  title_label->set_align (TextAlign::CENTER);

  double yoffset = 1;
  grid.add_widget (title_label, 0, yoffset, w + 2 * xframe, 2);
  yoffset += 2;

  grid.add_widget (new HLine (this, Color (0.6, 0.6, 0.6), 1), 0.5, yoffset, w + 2 * xframe - 1, 2);
  yoffset += 2;
  /* put each line in one label */
  for (const auto& line : lines)
    {
      auto line_label = new Label (this, line);

      line_label->set_align (TextAlign::LEFT);
      grid.add_widget (line_label, xframe, yoffset, w, 2);
      yoffset += 2;
    }
  grid.add_widget (new HLine (this, Color (0.6, 0.6, 0.6), 1), 0.5, yoffset, w + 2 * xframe - 1, 2);
  yoffset += 2;

  double xoffset = (w + 2 * xframe) / 2 - button_width / 2;
  for (auto b : bwidgets)
    {
      grid.add_widget (b, xoffset, yoffset, 10, 3);
      xoffset += 11;
    }

  yoffset += 3;

  grid.add_widget (this, 0, 0, w + 2 * xframe, yoffset + 1);
  window->set_keyboard_focus (this);
}

void
MessageBox::critical (Widget *parent, const string& title, const string& text)
{
  Dialog *dialog = new MessageBox (parent->window(), title, text, Buttons::OK);

  dialog->run();
}
