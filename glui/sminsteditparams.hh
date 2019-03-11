// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_PARAMS_HH
#define SPECTMORPH_INST_EDIT_PARAMS_HH

#include "smcheckbox.hh"

namespace SpectMorph
{

class InstEditParams : public Window
{
  Instrument *instrument = nullptr;

  CheckBox   *auto_volume_checkbox = nullptr;
  ComboBox   *auto_volume_method_combobox = nullptr;

  CheckBox   *auto_tune_checkbox = nullptr;
public:
  InstEditParams (Window *window, Instrument *instrument) :
    Window ("Instrument Params", 320, 400, 0, false, window->native_window()),
    instrument (instrument)
  {
    window->add_child_window (this);
    set_close_callback ([this,window]() { window->remove_child_window (this); });

    FixedGrid grid;
    auto_volume_checkbox = new CheckBox (this, "Auto Volume");
    connect (auto_volume_checkbox->signal_toggled, this, &InstEditParams::on_auto_volume_changed);
    grid.add_widget (auto_volume_checkbox, 2, 2, 20, 2);

    /*--- play mode ---*/
    auto_volume_method_combobox = new ComboBox (this);
    connect (auto_volume_method_combobox->signal_item_changed, this, &InstEditParams::on_auto_volume_method_changed);
    grid.add_widget (new Label (this, "Method"), 4, 4, 10, 3);
    grid.add_widget (auto_volume_method_combobox, 13, 4, 24, 3);
    auto_volume_method_combobox->add_item ("From Loop"); // default
    auto_volume_method_combobox->add_item ("Global");

    connect (instrument->signal_global_changed, this, &InstEditParams::on_global_changed);

    auto_tune_checkbox = new CheckBox (this, "Auto Tune");
    connect (auto_tune_checkbox->signal_toggled, this, &InstEditParams::on_auto_tune_changed);
    grid.add_widget (auto_tune_checkbox, 2, 7, 20, 2);

    on_global_changed();

    show();
  }
  void
  on_global_changed()
  {
    auto_volume_checkbox->set_checked (instrument->auto_volume().enabled);
    auto_tune_checkbox->set_checked (instrument->auto_tune().enabled);

    if (instrument->auto_volume().method == Instrument::AutoVolume::GLOBAL)
      auto_volume_method_combobox->set_text ("Global");
    else
      auto_volume_method_combobox->set_text ("From Loop");
  }
  void
  on_auto_volume_changed (bool new_value)
  {
    Instrument::AutoVolume av = instrument->auto_volume();
    av.enabled = new_value;

    instrument->set_auto_volume (av);
  }
  void
  on_auto_volume_method_changed()
  {
    Instrument::AutoVolume av = this->instrument->auto_volume();

    int idx = auto_volume_method_combobox->current_index();
    if (idx == 0)
      av.method = Instrument::AutoVolume::FROM_LOOP;
    else
      av.method = Instrument::AutoVolume::GLOBAL;

    this->instrument->set_auto_volume (av);
  }
  void
  on_auto_tune_changed (bool new_value)
  {
    auto at = instrument->auto_tune();
    at.enabled = new_value;

    instrument->set_auto_tune (at);
  }
};

}

#endif
