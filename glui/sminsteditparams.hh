// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_PARAMS_HH
#define SPECTMORPH_INST_EDIT_PARAMS_HH

#include "smcheckbox.hh"
#include "smparamlabel.hh"
#include "smbutton.hh"
#include "smshortcut.hh"

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
  ComboBox   *auto_tune_method_combobox = nullptr;
  Label      *auto_tune_method_label = nullptr;

  Label      *auto_tune_partials_label = nullptr;
  ParamLabel *auto_tune_partials_param_label = nullptr;

  Label      *auto_tune_time_label = nullptr;
  ParamLabel *auto_tune_time_param_label = nullptr;

  Label      *auto_tune_amount_label = nullptr;
  ParamLabel *auto_tune_amount_param_label = nullptr;

  CheckBox   *enc_cfg_checkbox = nullptr;
  std::vector<Widget *> enc_widgets;

  ScrollView *scroll_view = nullptr;
  Widget     *scroll_widget = nullptr;
public:
  InstEditParams (Window *window, Instrument *instrument) :
    Window (*window->event_loop(), "SpectMorph - Instrument Parameters", 320, 320, 0, false, window->native_window()),
    instrument (instrument)
  {
    window->add_child_window (this);
    set_close_callback ([this,window]() {
      signal_closed();
      window->remove_child_window (this);
     });

    Shortcut *play_shortcut = new Shortcut (this, ' ');
    connect (play_shortcut->signal_activated, [this]() { signal_toggle_play(); });

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

    auto gain_mod = new ParamLabelModelDouble (instrument->auto_volume().gain, -48.0, 48.0, "%.2f", "%.2f dB");
    auto_volume_gain_param_label = new ParamLabel (scroll_widget, gain_mod);

    connect (gain_mod->signal_value_changed, this, &InstEditParams::on_auto_volume_gain_changed);

    auto_tune_checkbox = new CheckBox (scroll_widget, "Auto Tune");
    connect (auto_tune_checkbox->signal_toggled, this, &InstEditParams::on_auto_tune_changed);

    /*--- auto tune method ---*/
    auto_tune_method_combobox = new ComboBox (scroll_widget);
    auto_tune_method_label = new Label (scroll_widget, "Method");

    connect (auto_tune_method_combobox->signal_item_changed, this, &InstEditParams::on_auto_tune_method_changed);
    auto_tune_method_combobox->add_item ("Simple"); // default
    auto_tune_method_combobox->add_item ("All Frames");
    auto_tune_method_combobox->add_item ("Smooth");

    /*--- auto tune partials ---*/
    auto_tune_partials_label = new Label (scroll_widget, "Partials");

    auto partials_mod = new ParamLabelModelInt (instrument->auto_tune().partials, 1, 3);
    auto_tune_partials_param_label = new ParamLabel (scroll_widget, partials_mod);

    connect (partials_mod->signal_value_changed, this, &InstEditParams::on_auto_tune_partials_changed);

    /*--- auto tune time ---*/
    auto_tune_time_label = new Label (scroll_widget, "Time");

    auto time_mod = new ParamLabelModelDouble (instrument->auto_tune().time, 1, 2000, "%.2f", "%.2f ms");
    auto_tune_time_param_label = new ParamLabel (scroll_widget, time_mod);

    connect (time_mod->signal_value_changed, this, &InstEditParams::on_auto_tune_time_changed);

    /*--- auto tune amount ---*/
    auto_tune_amount_label = new Label (scroll_widget, "Amount");

    auto amount_mod = new ParamLabelModelDouble (instrument->auto_tune().amount, 0, 100, "%.1f", "%.1f %%");
    auto_tune_amount_param_label = new ParamLabel (scroll_widget, amount_mod);

    connect (time_mod->signal_value_changed, this, &InstEditParams::on_auto_tune_amount_changed);

    /*--- encoder config ---*/
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
    const auto auto_tune = instrument->auto_tune();
    auto_tune_method_label->set_visible (auto_tune.enabled);
    auto_tune_method_combobox->set_visible (auto_tune.enabled);

    auto_tune_partials_label->set_visible (auto_tune.enabled &&
      (auto_tune.method == Instrument::AutoTune::ALL_FRAMES || auto_tune.method == Instrument::AutoTune::SMOOTH));
    auto_tune_partials_param_label->set_visible (auto_tune_partials_label->visible());

    auto_tune_time_label->set_visible (auto_tune.enabled && auto_tune.method == Instrument::AutoTune::SMOOTH);
    auto_tune_time_param_label->set_visible (auto_tune_time_label->visible());

    auto_tune_amount_label->set_visible (auto_tune.enabled && auto_tune.method == Instrument::AutoTune::SMOOTH);
    auto_tune_amount_param_label->set_visible (auto_tune_amount_label->visible());

    grid.add_widget (auto_tune_checkbox, 0, y, 20, 2);
    y += 2;
    if (auto_tune.enabled)
      {
        grid.add_widget (auto_tune_method_label, 2, y, 10, 3);
        grid.add_widget (auto_tune_method_combobox, 11, y, 23, 3);
        y += 3;
        if (auto_tune_partials_label->visible())
          {
            grid.add_widget (auto_tune_partials_label, 2, y, 10, 3);
            grid.add_widget (auto_tune_partials_param_label, 11, y, 10, 3);
            y += 3;
          }
        if (auto_tune_time_label->visible())
          {
            grid.add_widget (auto_tune_time_label, 2, y, 10, 3);
            grid.add_widget (auto_tune_time_param_label, 11, y, 10, 3);
            y += 3;
          }
        if (auto_tune_amount_label->visible())
          {
            grid.add_widget (auto_tune_amount_label, 2, y, 10, 3);
            grid.add_widget (auto_tune_amount_param_label, 11, y, 10, 3);
            y += 3;
          }
      }
    grid.add_widget (enc_cfg_checkbox, 0, y, 30, 2);
    y += 2;

    switch (auto_tune.method)
    {
      case Instrument::AutoTune::SIMPLE:
        auto_tune_method_combobox->set_text ("Simple");
        break;
      case Instrument::AutoTune::ALL_FRAMES:
        auto_tune_method_combobox->set_text ("All Frames");
        break;
      case Instrument::AutoTune::SMOOTH:
        auto_tune_method_combobox->set_text ("Smooth");
        break;
    }

    auto_tune_checkbox->set_checked (instrument->auto_tune().enabled);
    enc_cfg_checkbox->set_checked (instrument->encoder_config().enabled);

    if (instrument->auto_volume().method == Instrument::AutoVolume::GLOBAL)
      auto_volume_method_combobox->set_text ("Global");
    else
      auto_volume_method_combobox->set_text ("From Loop");

    auto encoder_config = instrument->encoder_config();

    for (auto w : enc_widgets) /* delete old enc widgets */
      w->delete_later();
    enc_widgets.clear();

    if (encoder_config.enabled)
      {
        for (size_t i = 0; i < encoder_config.entries.size(); i++)
          {
            auto param_mod = new ParamLabelModelString (encoder_config.entries[i].param);
            connect (param_mod->signal_value_changed, [this,i] (const std::string& s) { on_change_enc_entry (i, s.c_str(), nullptr); });

            auto value_mod = new ParamLabelModelString (encoder_config.entries[i].value);
            connect (value_mod->signal_value_changed, [this,i] (const std::string& s) { on_change_enc_entry (i, nullptr, s.c_str()); });

            ParamLabel *plabel = new ParamLabel (scroll_widget, param_mod);
            ParamLabel *vlabel = new ParamLabel (scroll_widget, value_mod);
            ToolButton *tbutton = new ToolButton (scroll_widget, 'x');

            grid.add_widget (plabel, 2, y, 18, 3);
            grid.add_widget (vlabel, 21, y, 11, 3);
            grid.add_widget (tbutton, 32.5, y + 0.5, 2, 2);
            y += 3;
            enc_widgets.push_back (plabel);
            enc_widgets.push_back (vlabel);
            enc_widgets.push_back (tbutton);

            connect (tbutton->signal_clicked, [this,i]() { on_remove_enc_entry (i); });
          }
        Button *add_button = new Button (scroll_widget, "Add Entry");
        grid.add_widget (add_button, 2, y, 12, 3);
        connect (add_button->signal_clicked, this, &InstEditParams::on_add_enc_entry);
        enc_widgets.push_back (add_button);
        y += 3;
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
    Instrument::AutoVolume av = instrument->auto_volume();

    int idx = auto_volume_method_combobox->current_index();
    if (idx == 0)
      av.method = Instrument::AutoVolume::FROM_LOOP;
    else
      av.method = Instrument::AutoVolume::GLOBAL;

    instrument->set_auto_volume (av);
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
  on_auto_tune_method_changed()
  {
    auto at = instrument->auto_tune();

    int idx = auto_tune_method_combobox->current_index();
    if (idx == 0)
      at.method = Instrument::AutoTune::SIMPLE;
    if (idx == 1)
      at.method = Instrument::AutoTune::ALL_FRAMES;
    if (idx == 2)
      at.method = Instrument::AutoTune::SMOOTH;

    instrument->set_auto_tune (at);
  }
  void
  on_auto_tune_partials_changed (int p)
  {
    auto at = instrument->auto_tune();
    at.partials = p;

    instrument->set_auto_tune (at);
  }
  void
  on_auto_tune_time_changed (double t)
  {
    auto at = instrument->auto_tune();
    at.time = t;

    instrument->set_auto_tune (at);
  }
  void
  on_auto_tune_amount_changed (double a)
  {
    auto at = instrument->auto_tune();
    at.amount = a;

    instrument->set_auto_tune (at);
  }
  void
  on_enc_cfg_changed (bool new_value)
  {
    auto enc_cfg = instrument->encoder_config();
    enc_cfg.enabled = new_value;

    instrument->set_encoder_config (enc_cfg);
  }
  void
  on_remove_enc_entry (size_t i)
  {
    auto enc_cfg = instrument->encoder_config();

    if (i < enc_cfg.entries.size())
      enc_cfg.entries.erase (enc_cfg.entries.begin() + i);

    instrument->set_encoder_config (enc_cfg);
  }
  void
  on_add_enc_entry()
  {
    auto enc_cfg = instrument->encoder_config();

    Instrument::EncoderEntry entry {"key", "value" };
    enc_cfg.entries.push_back (entry);

    instrument->set_encoder_config (enc_cfg);
  }
  void
  on_change_enc_entry (size_t i, const char *k, const char *v)
  {
    auto enc_cfg = instrument->encoder_config();

    if (i < enc_cfg.entries.size())
      {
        if (k)
          enc_cfg.entries[i].param = k;
        if (v)
          enc_cfg.entries[i].value = v;
      }

    instrument->set_encoder_config (enc_cfg);
  }
  Signal<> signal_toggle_play;
  Signal<> signal_closed;
};

}

#endif
