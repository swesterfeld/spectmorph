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
  title_label->set_bold (true);
  title_label->set_align (TextAlign::CENTER);

  grid.add_widget (title_label, 0, 0, 40, 4);
  grid.add_widget (this, 0, 0, 40, 15);

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

  connect (line_edit->signal_return_pressed,  [=]() {
    if (ok_button->enabled())
      on_accept();
  });
  connect (line_edit->signal_esc_pressed, this, &Dialog::on_reject);
  connect (ok_button->signal_clicked,     this, &Dialog::on_accept);
  connect (cancel_button->signal_clicked, this, &Dialog::on_reject);
}

string
RenameOpDialog::new_name()
{
  return line_edit->text();
}
