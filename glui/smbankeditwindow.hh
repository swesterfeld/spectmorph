// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smwindow.hh"
#include "smlistbox.hh"
#include "smcreatebankwindow.hh"

namespace SpectMorph
{

class BankEditWindow : public Window
{
  MorphWavSource *morph_wav_source;
  UserInstrumentIndex *user_instrument_index = nullptr;
  ListBox *list_box;
  std::vector<std::string> banks;
public:
  BankEditWindow (Window *parent_window, const std::string& title, MorphWavSource *morph_wav_source) :
    Window (*parent_window->event_loop(), title, 480, 320, 0, false, parent_window->native_window()),
    morph_wav_source (morph_wav_source)
  {
    user_instrument_index = morph_wav_source->morph_plan()->project()->user_instrument_index();

    FixedGrid grid;

    double yoffset = 1;
    list_box = new ListBox (this);
    grid.add_widget (list_box, 1, yoffset, 58, 26);
    yoffset += 26;

    auto delete_button = new Button (this, "Delete");
    connect (delete_button->signal_clicked, this, &BankEditWindow::on_delete_clicked);
    grid.add_widget (delete_button, 8, yoffset, 10, 3);

    auto create_bank_button = new Button (this, "Create Bank");
    connect (create_bank_button->signal_clicked, this, &BankEditWindow::on_create_bank_clicked);
    grid.add_widget (create_bank_button, 48, yoffset, 10, 3);

    connect (user_instrument_index->signal_banks_changed, this, &BankEditWindow::on_banks_changed);

    on_banks_changed();
    show();
  }
  void
  on_banks_changed()
  {
    banks.clear();
    list_box->clear();
    for (auto bank : morph_wav_source->list_banks())
      {
        list_box->add_item (string_printf ("%s (%d)", bank.c_str(), user_instrument_index->count (bank)));
        banks.push_back (bank);
      }
  }
  void
  on_delete_clicked()
  {
    int item = list_box->selected_item();
    if (item >= 0 && item < int (banks.size()))
      {
        std::string bank = banks[item];
        auto confirm_box = new MessageBox (window(),
          string_printf ("Delete Bank '%s'?", bank.c_str()),
          string_printf ("This bank contains %d instruments.\n"
                         "\n"
                         "If you delete the bank, these instruments will be deleted.\n", user_instrument_index->count (bank)),
                         MessageBox::DELETE | MessageBox::CANCEL);
        confirm_box->run ([this, bank](bool delete_bank)
          {
            if (delete_bank)
              user_instrument_index->remove_bank (bank);
          });
      }
  }
  void
  on_create_bank_clicked()
  {
    auto cwin = new CreateBankWindow (window(), user_instrument_index);

    // after this line, rename window is owned by parent window
    window()->set_popup_window (cwin);
  }
};

}
