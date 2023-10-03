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
  std::map<int, Label *> inst_label;
  std::string active_bank;
public:
  BankEditWindow (Window *parent_window, const std::string& title, MorphWavSource *morph_wav_source) :
    Window (*parent_window->event_loop(), title, 768, 67 * 8, 0, false, parent_window->native_window()),
    morph_wav_source (morph_wav_source)
  {
    user_instrument_index = morph_wav_source->morph_plan()->project()->user_instrument_index();

    FixedGrid grid;

    double yoffset = 1;

    auto create_bank_button = new Button (this, "Create Bank");
    connect (create_bank_button->signal_clicked, this, &BankEditWindow::on_create_bank_clicked);
    grid.add_widget (create_bank_button, 1, yoffset, 20, 3);
    yoffset += 3;

    list_box = new ListBox (this);
    connect (list_box->signal_selected_item_changed, this, &BankEditWindow::on_selected_item_changed);
    grid.add_widget (list_box, 1, yoffset, 20, 59);
    yoffset += 59;

    auto delete_button = new Button (this, "Delete Bank");
    connect (delete_button->signal_clicked, this, &BankEditWindow::on_delete_clicked);
    grid.add_widget (delete_button, 1, yoffset, 20, 3);

    yoffset += 3;

    connect (user_instrument_index->signal_banks_changed, this, &BankEditWindow::on_banks_changed);

    int i = 1;
    for (int x = 0; x < 8; x++)
      for (int y = 0; y < 22; y++)
        {
          if (i <= 128)
            {
              auto clickable_label = new PropertyViewLabel (this, "");
              connect (clickable_label->signal_clicked, [this, i] () {
                signal_instrument_clicked (active_bank, i);
              });
              inst_label[i] = clickable_label;
              grid.add_widget (clickable_label, 22 + 12.5 * x, 1 + 3 * y, 11.2, 2);
              i++;
            }
        }
    for (int x = 1; x < 6; x++)
      grid.add_widget (new VLine (this, Color (0.45, 0.45, 0.45), 1), 21 + 12.5 * x, 1, 1, 65);

    on_banks_changed();
    for (size_t b = 0; b < banks.size(); b++)
      if (banks[b] == morph_wav_source->bank())
        list_box->set_selected_item (b);
    on_bank_changed();
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
    if (banks.size())
      list_box->set_selected_item (0);
  }
  void
  on_selected_item_changed()
  {
    int item = list_box->selected_item();
    if (item >= 0 && item < int (banks.size()))
      {
        active_bank = banks[item];
        on_bank_changed();
      }
  }
  void
  on_bank_changed()
  {
    for (int i = 1; i <= 128; i++)
      inst_label[i]->set_text (user_instrument_index->label (active_bank, i));
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
  Signal<std::string, int> signal_instrument_clicked;
};

}
