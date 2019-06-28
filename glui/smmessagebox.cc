// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmessagebox.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smbutton.hh"
#include "smsimplelines.hh"

using std::vector;
using std::string;
using std::max;

using namespace SpectMorph;

static vector<string>
split (const string& text)
{
  vector<string> lines;

  string s;
  for (char c : text)
    {
      if (c == '\n')
        {
          lines.push_back (s);
          s = "";
        }
      else
        {
          s += c;
        }
    }
  if (s != "")
    lines.push_back (s);
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

  // window width
  double w = max (20.0, button_width);
  for (auto line : split (text))
    w = max (w, DrawUtils::static_text_extents (window, line).x_advance / 8);

  auto title_label = new Label (this, title);
  title_label->set_bold (true);
  title_label->set_align (TextAlign::CENTER);

  const double xframe = 2;

  double yoffset = 1;
  grid.add_widget (title_label, 0, yoffset, w + 2 * xframe, 2);
  yoffset += 2;

  grid.add_widget (new HLine (this, Color (0.6, 0.6, 0.6), 1), 0.5, yoffset, w + 2 * xframe - 1, 2);
  yoffset += 2;
  /* put each line in one label */
  for (auto line : split (text))
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
