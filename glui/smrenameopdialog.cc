// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smrenameopdialog.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smlineedit.hh"
#include "smbutton.hh"

using namespace SpectMorph;

using std::string;

RenameOpDialog::RenameOpDialog (Window *window, MorphOperator *op) :
  Dialog (window)
{
  FixedGrid grid;

  auto title_label = new Label (this, "Rename");
  title_label->bold = true;
  title_label->align = TextAlign::CENTER;

  grid.add_widget (title_label, 0, 0, 40, 4);
  grid.add_widget (this, 0, 0, 40, 15);
  x = (window->width - width) / 2;
  y = (window->height - height) / 2;

  double yoffset = 4;
  grid.add_widget (new Label (this, "Old Name"), 3, yoffset, 30, 3);
  grid.add_widget (new Label (this, op->name()), 15, yoffset, 20, 3);
  yoffset += 3;
  grid.add_widget (new Label (this, "New Name"), 3, yoffset, 30, 3);

  line_edit = new LineEdit (this, op->name());
  grid.add_widget (line_edit, 15, yoffset, 20, 3);
  window->set_keyboard_focus (line_edit);

  yoffset += 3;
  yoffset++;

  ok_button = new Button (this, "Ok");
  cancel_button = new Button (this, "Cancel");

  grid.add_widget (ok_button, 18, yoffset, 10, 3);
  grid.add_widget (cancel_button, 28, yoffset, 10, 3);

  connect (line_edit->signal_text_changed, [=](string txt) {
    ok_button->set_enabled (op->can_rename (txt));
  });
}

string
RenameOpDialog::new_name()
{
  return line_edit->text();
}

void
RenameOpDialog::run (std::function<void(bool)> callback)
{
  window()->set_dialog_widget (this);

  connect (line_edit->signal_return_pressed,  [=]() { callback (ok_button->enabled()); delete this; });
  connect (line_edit->signal_esc_pressed,     [=]() { callback (false); delete this; });
  connect (ok_button->signal_clicked,         [=]() { callback (true); delete this; });
  connect (cancel_button->signal_clicked,     [=]() { callback (false); delete this; });
}
