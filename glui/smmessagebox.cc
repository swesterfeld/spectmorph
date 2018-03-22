// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmessagebox.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smbutton.hh"

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


MessageBox::MessageBox (Window *window, const string& title, const string& text) :
  Dialog (window)
{
  FixedGrid grid;

  // window width
  double w = 20;
  for (auto line : split (text))
    w = max (w, DrawUtils::static_text_width (window, line) / 8);

  auto title_label = new Label (this, title);
  title_label->set_bold (true);
  title_label->set_align (TextAlign::CENTER);

  const double xframe = 2;

  grid.add_widget (title_label, 0, 0, w + 2 * xframe, 4);

  double yoffset = 4;
  /* put each line in one label */
  for (auto line : split (text))
    {
      auto line_label = new Label (this, line);

      line_label->set_align (TextAlign::CENTER);
      grid.add_widget (line_label, xframe, yoffset, w, 2);
      yoffset += 2;
    }
  yoffset += 1;

  auto ok_button = new Button (this, "Ok");
  grid.add_widget (ok_button, (w + 2 * xframe) / 2 - 5, yoffset, 10, 3);
  connect (ok_button->signal_clicked, this, &Dialog::on_accept);
  yoffset += 3;

  grid.add_widget (this, 0, 0, w + 2 * xframe, yoffset + 1);
  window->set_keyboard_focus (this);
}

void
MessageBox::critical (Widget *parent, const string& title, const string& text)
{
  Dialog *dialog = new MessageBox (parent->window(), title, text);

  dialog->run();
}
