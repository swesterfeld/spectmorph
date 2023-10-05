// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smdialog.hh"

namespace SpectMorph
{

class CreateBankWindow : public Window
{
  LineEdit             *line_edit = nullptr;
  Window               *parent_window = nullptr;
  UserInstrumentIndex  *user_instrument_index = nullptr;
public:
  CreateBankWindow (Window *window, UserInstrumentIndex *user_instrument_index) :
    Window (*window->event_loop(), "Create Bank", 320, 12 * 8, 0, false, window->native_window()),
    parent_window (window),
    user_instrument_index (user_instrument_index)
  {
    FixedGrid grid;

    double yoffset = 1;
    grid.add_widget (new Label (this, "Bank names can contain letters, digits, spaces, '-' or '_'"), 1, yoffset, 38, 3);

    yoffset += 3;

    grid.add_widget (new Label (this, "Name"), 1, yoffset, 30, 3);

    line_edit = new LineEdit (this, "");
    grid.add_widget (line_edit, 7, yoffset, 31, 3);
    set_keyboard_focus (line_edit);

    yoffset += 3;
    yoffset++;

    auto ok_button = new Button (this, "Ok");
    auto cancel_button = new Button (this, "Cancel");

    grid.add_widget (ok_button, 17, yoffset, 10, 3);
    grid.add_widget (cancel_button, 28, yoffset, 10, 3);

    connect (line_edit->signal_text_changed, [=](const std::string& txt) {
      ok_button->set_enabled (check_name());
    });
    ok_button->set_enabled (check_name());

    connect (line_edit->signal_return_pressed,  [=]() {
      if (ok_button->enabled())
        on_accept();
    });
    connect (line_edit->signal_esc_pressed, this, &CreateBankWindow::on_reject);
    connect (ok_button->signal_clicked,     this, &CreateBankWindow::on_accept);
    connect (cancel_button->signal_clicked, this, &CreateBankWindow::on_reject);

    set_close_callback ([this]() { on_reject(); });

    show();
  }
  void
  on_reject()
  {
    parent_window->set_popup_window (nullptr); // close this window
  }
  void
  on_accept()
  {
    auto bank = line_edit->text();
    Error error = user_instrument_index->create_bank (bank);

    if (error)
      signal_create_bank_error (bank, error);

    parent_window->set_popup_window (nullptr); // close this window
  }
  bool
  check_name()
  {
    for (auto ch : line_edit->text())
      {
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
             ch == ' ' || ch == '-' || ch == '_')
          {
            // fine
          }
        else
          {
            return false;
          }
      }
    return !line_edit->text().empty();
  }
  Signal<std::string, Error> signal_create_bank_error;
};

}
