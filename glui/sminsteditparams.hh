// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_PARAMS_HH
#define SPECTMORPH_INST_EDIT_PARAMS_HH

#include "smcheckbox.hh"
#include "smparamlabel.hh"
#include "smbutton.hh"

namespace SpectMorph
{

class InstEditParams : public Window
{
  Instrument *instrument = nullptr;

  CheckBox   *auto_volume_checkbox = nullptr;
  ComboBox   *auto_volume_method_combobox = nullptr;
  Label      *auto_volume_method_label = nullptr;
  Label      *auto_volume_gain_label = nullptr;
  ParamLabel *auto_volume_gain_param_label = nullptr;

  CheckBox   *auto_tune_checkbox = nullptr;

  CheckBox   *enc_cfg_checkbox = nullptr;
  std::vector<Widget *> enc_widgets;

  ScrollView *scroll_view = nullptr;
  Widget     *scroll_widget = nullptr;
public:
  InstEditParams (Window *window, Instrument *instrument) :
    Window ("Instrument Params", 320, 400, 0, false, window->native_window()),
    instrument (instrument)
  {
    window->add_child_window (this);
    set_close_callback ([this,window]() { window->remove_child_window (this); });

    FixedGrid grid;
    grid.add_widget (scroll_view = new ScrollView (this), 1, 1, width / 8 - 2, height / 8 - 2);
    scroll_widget = new Widget (scroll_view);
    scroll_view->set_scroll_widget (scroll_widget, false, true);

    auto_volume_checkbox = new CheckBox (scroll_widget, "Auto Volume");
    connect (auto_volume_checkbox->signal_toggled, this, &InstEditParams::on_auto_volume_changed);

    /*--- auto volume method ---*/
    auto_volume_method_combobox = new ComboBox (scroll_widget);
    auto_volume_method_label = new Label (scroll_widget, "Method");

    connect (auto_volume_method_combobox->signal_item_changed, this, &InstEditParams::on_auto_volume_method_changed);
    auto_volume_method_combobox->add_item ("From Loop"); // default
    auto_volume_method_combobox->add_item ("Global");

    /*--- auto volume gain ---*/
    auto_volume_gain_label = new Label (scroll_widget, "Gain");

    auto_volume_gain_param_label = new ParamLabel (scroll_widget, "");

    connect (auto_volume_gain_param_label->signal_value_changed, this, &InstEditParams::on_auto_volume_gain_changed);

    auto_tune_checkbox = new CheckBox (scroll_widget, "Auto Tune");
    connect (auto_tune_checkbox->signal_toggled, this, &InstEditParams::on_auto_tune_changed);

    enc_cfg_checkbox = new CheckBox (scroll_widget, "Custom Analysis Parameters");
    connect (enc_cfg_checkbox->signal_toggled, this, &InstEditParams::on_enc_cfg_changed);


    connect (instrument->signal_global_changed, this, &InstEditParams::on_global_changed);
    on_global_changed();
    show();
  }
  void
  on_global_changed()
  {
    FixedGrid grid;

    const auto auto_volume = instrument->auto_volume();
    auto_volume_checkbox->set_checked (auto_volume.enabled);
    auto_volume_method_label->set_visible (auto_volume.enabled);
    auto_volume_method_combobox->set_visible (auto_volume.enabled);
    auto_volume_gain_label->set_visible (auto_volume.enabled && auto_volume.method == Instrument::AutoVolume::GLOBAL);
    auto_volume_gain_param_label->set_visible (auto_volume.enabled && auto_volume.method == Instrument::AutoVolume::GLOBAL);

    double y = 0;
    grid.add_widget (auto_volume_checkbox, 0, y, 20, 2);
    y += 2;
    if (auto_volume.enabled)
      {
        grid.add_widget (auto_volume_method_label, 2, y, 10, 3);
        grid.add_widget (auto_volume_method_combobox, 11, y, 23, 3);
        y += 3;
        if (auto_volume.method == Instrument::AutoVolume::GLOBAL)
          {
            grid.add_widget (auto_volume_gain_label, 2, y, 10, 3);
            grid.add_widget (auto_volume_gain_param_label, 11, y, 10, 3);
            y += 3;
          }
      }
    grid.add_widget (auto_tune_checkbox, 0, y, 20, 2);
    y += 2;
    grid.add_widget (enc_cfg_checkbox, 0, y, 30, 2);
    y += 2;

    auto_tune_checkbox->set_checked (instrument->auto_tune().enabled);
    enc_cfg_checkbox->set_checked (instrument->encoder_config().enabled);
    auto_volume_gain_param_label->set_text (string_printf ("%.2f dB", instrument->auto_volume().gain));

    if (instrument->auto_volume().method == Instrument::AutoVolume::GLOBAL)
      auto_volume_method_combobox->set_text ("Global");
    else
      auto_volume_method_combobox->set_text ("From Loop");

    auto encoder_config = instrument->encoder_config();

    for (auto w : enc_widgets) /* delete old enc widgets */
      delete w;
    enc_widgets.clear();

    if (encoder_config.enabled)
      {
        for (int i = 0; i < encoder_config.entries.size(); i++)
          {
            ParamLabel *plabel = new ParamLabel (scroll_widget, encoder_config.entries[i].param);
            ParamLabel *vlabel = new ParamLabel (scroll_widget, encoder_config.entries[i].value);
            ToolButton *tbutton = new ToolButton (scroll_widget, 'x');

            grid.add_widget (plabel, 2, y, 20, 3);
            grid.add_widget (vlabel, 21, y, 11, 3);
            grid.add_widget (tbutton, 32.5, y + 0.5, 2, 2);
            y += 3;
            enc_widgets.push_back (plabel);
            enc_widgets.push_back (vlabel);
            enc_widgets.push_back (tbutton);
          }
      }

    scroll_widget->height = y * 8;
    scroll_widget->width = 32 * 8;
    scroll_view->on_widget_size_changed();
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
  on_auto_volume_gain_changed (double gain)
  {
    Instrument::AutoVolume av = instrument->auto_volume();
    av.gain = gain;

    instrument->set_auto_volume (av);
  }
  void
  on_auto_tune_changed (bool new_value)
  {
    auto at = instrument->auto_tune();
    at.enabled = new_value;

    instrument->set_auto_tune (at);
  }
  void
  on_enc_cfg_changed (bool new_value)
  {
    auto enc_cfg = instrument->encoder_config();
    enc_cfg.enabled = new_value;

    instrument->set_encoder_config (enc_cfg);
  }
};

}

#endif
