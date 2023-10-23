// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smslider.hh"
#include "smled.hh"
#include "smaudiotool.hh"

namespace SpectMorph
{

class InstEditVolume : public Window
{
  Instrument     *instrument = nullptr;
  SynthInterface *synth_interface = nullptr;
  ScrollView     *scroll_view = nullptr;
  Widget         *scroll_widget = nullptr;
  Button         *reset_button = nullptr;
  Slider         *play_volume_slider = nullptr;
  Label          *play_volume_value_label = nullptr;

  struct VolumeEdit : public Widget {
    Sample *sample = nullptr;
    Label  *db_label = nullptr;
    Led    *led = nullptr;
    Button *play_button = nullptr;
    Slider *slider = nullptr;
    Label  *energy_delta_label = nullptr;
    VolumeEdit (Widget *parent, Instrument *instrument, Sample *sample) :
      Widget (parent),
      sample (sample)
    {
      FixedGrid grid;
      double y = 0;

      db_label = new Label (this, "");
      db_label->set_orientation (Orientation::VERTICAL);
      grid.add_widget (db_label, 0, y, 3, 4);
      y += 4;

      led = new Led (this);
      grid.add_widget (led, 0.5, y + 0.5, 2, 2);
      y += 3;

      slider = new Slider (this, 0, Orientation::VERTICAL);
      grid.add_widget (slider, 0, y, 3, 20);
      y += 20;
      connect (slider->signal_value_changed,
               [sample] (double value) { sample->set_volume (slider_to_volume (value, sample_min_db, sample_max_db)); });

      Label *label = new Label (this, note_to_text (sample->midi_note()));
      label->set_orientation (Orientation::VERTICAL);
      grid.add_widget (label, 0, y, 3, 4);
      y += 5;

      play_button = new Button (this, "P");
      play_button->set_right_press (true);
      grid.add_widget (play_button, 0, y, 3, 3);
      y += 3;

      energy_delta_label = new Label (this, "");
      energy_delta_label->set_orientation (Orientation::VERTICAL);
      grid.add_widget (energy_delta_label, 0, y, 3, 4);
      y += 5;

      connect (instrument->signal_volume_changed, this, &VolumeEdit::on_volume_changed);
      on_volume_changed();
    }
    void
    on_volume_changed()
    {
      db_label->set_text (string_printf ("%.1f", sample->volume()));
      slider->set_value (volume_to_slider (sample->volume(), sample_min_db, sample_max_db));
    }
  };
  std::vector<VolumeEdit *> sample_widgets;

  Index     inst_index;
  ComboBox *ref_inst_combobox = nullptr;

  static constexpr double global_min_db = -12;
  static constexpr double global_max_db = 36;
  static constexpr double sample_min_db = -12;
  static constexpr double sample_max_db = 12;
  static constexpr double play_min_db = -36;
  static constexpr double play_max_db = 12;
public:
  InstEditVolume (Window *window, Instrument *instrument, SynthInterface *synth_interface, const std::string& reference, bool midi_to_reference, float play_gain) :
    Window (*window->event_loop(), "SpectMorph - Instrument Volume Editor", 64 * 8, 52 * 8, 0, false, window->native_window()),
    instrument (instrument),
    synth_interface (synth_interface)
  {
    set_close_callback ([this]() {
      signal_closed();
      delete_later();
     });

    FixedGrid grid;

    Label  *global_volume_label = new Label (this, "");
    global_volume_label->set_align (TextAlign::CENTER);
    auto update_global_volume_label = [global_volume_label, instrument]() { global_volume_label->set_text (string_printf ("%.1f dB", instrument->global_volume())); };
    connect (instrument->signal_volume_changed, update_global_volume_label);
    update_global_volume_label();

    grid.add_widget (global_volume_label, 1, 1, 6, 3);

    Slider *global_slider = new Slider (this, volume_to_slider (instrument->global_volume(), global_min_db, global_max_db), Orientation::VERTICAL);
    grid.add_widget (global_slider, 2, 4, 3, 40);
    connect (global_slider->signal_value_changed,
             [this] (double value) { this->instrument->set_global_volume (slider_to_volume (value, global_min_db, global_max_db)); });

    scroll_view = new ScrollView (this);
    grid.add_widget (scroll_view, 7, 1, 52, 43);

    scroll_widget = new Widget (scroll_view);
    scroll_view->set_scroll_widget (scroll_widget, true, false);

    reset_button = new Button (this, "Reset to Loop Energy");
    connect (reset_button->signal_clicked, this, &InstEditVolume::reset_to_loop_energy);
    grid.add_widget (reset_button, 1, 45, 20, 3);

    ref_inst_combobox = new ComboBox (this);
    grid.add_widget (ref_inst_combobox, 1, 48, 20, 3);

    inst_index.load_file ("instruments:standard");

    for (auto group : inst_index.groups())
      {
        ref_inst_combobox->add_item (ComboBoxItem (group.group, true));
        for (auto instrument : group.instruments)
          {
            ref_inst_combobox->add_item (ComboBoxItem (instrument.label));
          }
      }
    ref_inst_combobox->set_text (inst_index.smset_to_label (reference));
    connect  (ref_inst_combobox->signal_item_changed, [this]() {
      std::string smset = inst_index.label_to_smset (ref_inst_combobox->text());
      signal_reference_changed (smset);
    });

    CheckBox *midi_to_reference_checkbox = new CheckBox (this, "Use Reference for Midi Input");
    midi_to_reference_checkbox->set_checked (midi_to_reference);
    grid.add_widget (midi_to_reference_checkbox, 22, 48.5, 20, 2);
    connect (midi_to_reference_checkbox->signal_toggled, [this] (bool checked) {
      signal_midi_to_reference_changed (checked);
    });
    Shortcut *midi_to_ref_shortcut = new Shortcut (this, ' ');
    connect (midi_to_ref_shortcut->signal_activated, [midi_to_reference_checkbox, this]() {
      midi_to_reference_checkbox->set_checked (!midi_to_reference_checkbox->checked());
      signal_midi_to_reference_changed (midi_to_reference_checkbox->checked());
    });

    double play_db = db_from_factor (play_gain, -96);
    play_volume_slider = new Slider (this, volume_to_slider (play_db, play_min_db, play_max_db));
    play_volume_value_label = new Label (this, "");
    on_play_volume_changed (play_volume_slider->value());

    connect (play_volume_slider->signal_value_changed, this, &InstEditVolume::on_play_volume_changed);

    grid.add_widget (new Label (this, "Play Volume"), 22, 46, 10, 2);
    grid.add_widget (play_volume_slider, 30, 46, 23, 2);
    grid.add_widget (play_volume_value_label, 54, 46, 7, 2);

    connect (instrument->signal_samples_changed, this, &InstEditVolume::on_samples_changed);
    on_samples_changed();
    audio_updated();
    show();
  }
  static double
  volume_to_slider (double volume, double min, double max)
  {
    return std::clamp ((volume - min) / (max - min), 0.0, 1.0);
  }
  static double
  slider_to_volume (double value, double min, double max)
  {
    return std::clamp (min + value * (max - min), min, max);
  }
  void
  on_play_volume_changed (double value)
  {
    double volume_db = slider_to_volume (value, play_min_db, play_max_db);
    play_volume_value_label->set_text (string_printf ("%.1f dB", volume_db));
    signal_gain_changed (db_to_factor (volume_db));
  }
  void
  on_samples_changed()
  {
    FixedGrid grid;
    // remove old sample widgets created before
    for (auto w : sample_widgets)
      w->delete_later();
    sample_widgets.clear();

    std::vector<Sample *> samples;
    for (size_t i = 0; i < instrument->size(); i++)
      samples.push_back (instrument->sample (i));
    std::reverse (samples.begin(), samples.end()); // instrument samples start with highest note

    double x = 0;
    for (auto sample : samples)
      {
        VolumeEdit *volume_edit = new VolumeEdit (scroll_widget, instrument, sample);
        grid.add_widget (volume_edit, x, 0, 3, 30);
        sample_widgets.push_back (volume_edit);
        x += 3;

        int note = sample->midi_note();
        connect (volume_edit->play_button->signal_pressed, [this, note]() {
          synth_interface->synth_inst_edit_note (note, true, 0);
        });
        connect (volume_edit->play_button->signal_right_pressed, [this, note]() {
          synth_interface->synth_inst_edit_note (note, true, 2);
        });
        connect (volume_edit->play_button->signal_released, [this, note]() {
          synth_interface->synth_inst_edit_note (note, false, 0);
        });
        connect (volume_edit->play_button->signal_right_released, [this, note]() {
          synth_interface->synth_inst_edit_note (note, false, 2);
        });
      }
    scroll_widget->set_height (30 * 8);
    scroll_widget->set_width (x * 8);
    scroll_view->on_widget_size_changed();
  }
  static std::string
  note_to_text (int i) /* FIXME: dedup? */
  {
    std::vector<std::string> note_name { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return string_printf ("%s%d", note_name[i % 12].c_str(), i / 12 - 2);
  }
  void
  set_active_notes (const std::vector<int>& notes)
  {
    for (auto volume_edit : sample_widgets)
      {
        bool on = std::find (notes.begin(), notes.end(), volume_edit->sample->midi_note()) != notes.end();
        volume_edit->led->set_on (on);
      }
  }
  void
  set_analyzing (bool analyzing)
  {
    reset_button->set_enabled (!analyzing);
  }
  void
  audio_updated()
  {
    for (size_t i = 0; i < instrument->size(); i++)
      {
        Sample *sample = instrument->sample (i);
        if (sample->audio)
          {
            const double energy = AudioTool::compute_energy (*sample->audio);
            const double target_energy = 0.05;
            const double factor = sqrt (energy / target_energy);
            for (auto volume_edit : sample_widgets)
              {
                if (volume_edit->sample->midi_note() == sample->midi_note())
                  volume_edit->energy_delta_label->set_text (string_printf ("%.1f", db_from_factor (factor, -96)));
              }
          }
      }
  }
  void
  reset_to_loop_energy()
  {
    std::vector<double> new_volume;
    for (size_t i = 0; i < instrument->size(); i++)
      {
        Sample *sample = instrument->sample (i);
        if (sample->audio)
          {
            const double energy = AudioTool::compute_energy (*sample->audio);
            const double target_energy = 0.05;
            const double factor = sqrt (energy / target_energy);
            new_volume.push_back (sample->volume() - db_from_factor (factor, -96));
          }
      }
    /* if we had nullptr samples, analysis was still running */
    if (new_volume.size() == instrument->size())
      for (size_t i = 0; i < instrument->size(); i++)
        instrument->sample (i)->set_volume (new_volume[i]);
  }
  Signal<>            signal_toggle_play;
  Signal<>            signal_closed;
  Signal<std::string> signal_reference_changed;
  Signal<bool>        signal_midi_to_reference_changed;
  Signal<float>       signal_gain_changed;
};

}
