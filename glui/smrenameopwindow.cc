// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smrenameopwindow.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smlineedit.hh"
#include "smbutton.hh"

using namespace SpectMorph;

using std::string;

void
RenameOpWindow::create (Window *window, MorphOperator *op)
{
  Window *rwin = new RenameOpWindow (window, op);

  // after this line, rename window is owned by parent window
  window->set_popup_window (rwin);
}

RenameOpWindow::RenameOpWindow (Window *window, MorphOperator *op) :
  Window (*window->event_loop(), "Rename", 320, 88, 0, false, window->native_window())
{
  parent_window = window;
  m_op = op;

  FixedGrid grid;

  double yoffset = 2;
  grid.add_widget (new Label (this, "Name"), 1, yoffset, 30, 3);

  line_edit = new LineEdit (this, op->name());
  line_edit->select_all();
  grid.add_widget (line_edit, 7, yoffset, 31, 3);
  set_keyboard_focus (line_edit);

  yoffset += 3;
  yoffset++;

  ok_button = new Button (this, "Ok");
  cancel_button = new Button (this, "Cancel");

  grid.add_widget (ok_button, 17, yoffset, 10, 3);
  grid.add_widget (cancel_button, 28, yoffset, 10, 3);

  connect (line_edit->signal_text_changed, [=](string txt) {
    ok_button->set_enabled (op->can_rename (txt));
  });

  connect (line_edit->signal_return_pressed,  [=]() {
    if (ok_button->enabled())
      on_accept();
  });
  connect (line_edit->signal_esc_pressed, this, &RenameOpWindow::on_reject);
  connect (ok_button->signal_clicked,     this, &RenameOpWindow::on_accept);
  connect (cancel_button->signal_clicked, this, &RenameOpWindow::on_reject);

  set_close_callback ([this]() { on_reject(); });

  show();
}

void
RenameOpWindow::on_accept()
{
  m_op->set_name (line_edit->text());

  parent_window->set_popup_window (nullptr); // close this window
}

void
RenameOpWindow::on_reject()
{
  parent_window->set_popup_window (nullptr); // close this window
}
