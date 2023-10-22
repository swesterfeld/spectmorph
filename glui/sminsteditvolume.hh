// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smslider.hh"
#include "smled.hh"

namespace SpectMorph
{

class InstEditVolume : public Window
{
  Instrument     *instrument = nullptr;
  SynthInterface *synth_interface = nullptr;
  ScrollView     *scroll_view = nullptr;
  Widget         *scroll_widget = nullptr;

  std::vector<Widget *> sample_widgets;

  struct NoteLed {
    int note = 0;
    Led *led = nullptr;
  };
  std::vector<NoteLed> note_leds;

  static constexpr double global_min_db = -12;
  static constexpr double global_max_db = 36;
  static constexpr double sample_min_db = -12;
  static constexpr double sample_max_db = 12;
public:
  InstEditVolume (Window *window, Instrument *instrument, SynthInterface *synth_interface) :
    Window (*window->event_loop(), "SpectMorph - Instrument Volume Editor", 64 * 8, 9 * 40 + 6 * 8, 0, false, window->native_window()),
    instrument (instrument),
    synth_interface (synth_interface)
  {
    set_close_callback ([this]() {
      signal_closed();
      delete_later();
     });

    Shortcut *play_shortcut = new Shortcut (this, ' ');
    connect (play_shortcut->signal_activated, [this]() { signal_toggle_play(); });

    FixedGrid grid;

    Label  *global_volume_label = new Label (this, "");
    global_volume_label->set_align (TextAlign::CENTER);
    auto update_global_volume_label = [global_volume_label, instrument]() { global_volume_label->set_text (string_printf ("%.1f dB", instrument->global_volume())); };
    update_global_volume_label();
    connect (instrument->signal_volume_changed, update_global_volume_label);

    grid.add_widget (global_volume_label, 1, 1, 6, 3);

    Slider *global_slider = new Slider (this, volume_to_slider (instrument->global_volume(), global_min_db, global_max_db), Orientation::VERTICAL);
    grid.add_widget (global_slider, 2, 4, 3, 42);
    connect (global_slider->signal_value_changed,
             [this] (double value) { this->instrument->set_global_volume (slider_to_volume (value, global_min_db, global_max_db)); });

    scroll_view = new ScrollView (this);
    grid.add_widget (scroll_view, 7, 1, 52, 46);

    scroll_widget = new Widget (scroll_view);
    scroll_view->set_scroll_widget (scroll_widget, true, false);

    connect (instrument->signal_samples_changed, this, &InstEditVolume::on_samples_changed);
    on_samples_changed();
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
  on_samples_changed()
  {
    FixedGrid grid;
    // remove old sample widgets created before
    for (auto w : sample_widgets)
      w->delete_later();
    sample_widgets.clear();
    note_leds.clear();

    std::vector<Sample *> samples;
    for (size_t i = 0; i < instrument->size(); i++)
      samples.push_back (instrument->sample (i));
    std::reverse (samples.begin(), samples.end()); // instrument samples start with highest note

    double x = 0;
    for (auto sample : samples)
      {
        double y = 1;
        Label *db_label = new Label (scroll_widget, "");
        db_label->set_orientation (Orientation::VERTICAL);
        grid.add_widget (db_label, x, y, 3, 4);

        auto update_db_label = [db_label, sample]() { db_label->set_text (string_printf ("%.1f", sample->volume())); };
        connect (instrument->signal_volume_changed, update_db_label);
        update_db_label();
        y += 4;

        NoteLed note_led;
        note_led.note = sample->midi_note();
        note_led.led = new Led (scroll_widget);
        note_leds.push_back (note_led);
        grid.add_widget (note_led.led, x + 0.5, y + 0.5, 2, 2);
        y += 3;

        Slider *slider = new Slider (scroll_widget, volume_to_slider (sample->volume(), sample_min_db, sample_max_db), Orientation::VERTICAL);
        grid.add_widget (slider, x, y, 3, 20);
        y += 20;
        sample_widgets.push_back (slider);
        connect (slider->signal_value_changed,
                 [sample] (double value) { sample->set_volume (slider_to_volume (value, sample_min_db, sample_max_db)); });

        Label *label = new Label (scroll_widget, note_to_text (sample->midi_note()));
        label->set_orientation (Orientation::VERTICAL);
        grid.add_widget (label, x, y, 3, 4);
        sample_widgets.push_back (label);
        y += 5;

        Button *button = new Button (scroll_widget, "P");
        button->set_right_press (true);
        grid.add_widget (button, x, y, 3, 3);
        sample_widgets.push_back (button);
        y += 3;

        x += 3;
        int note = sample->midi_note();
        connect (button->signal_pressed, [this, note]() {
          synth_interface->synth_inst_edit_note (note, true, 0);
        });
        connect (button->signal_right_pressed, [this, note]() {
          synth_interface->synth_inst_edit_note (note, true, 2);
        });
        connect (button->signal_released, [this, note]() {
          synth_interface->synth_inst_edit_note (note, false, 0);
        });
        connect (button->signal_right_released, [this, note]() {
          synth_interface->synth_inst_edit_note (note, false, 2);
        });
      }
    scroll_widget->set_height (30 * 8);
    scroll_widget->set_width (x * 8);
    scroll_view->on_widget_size_changed();
  }
  std::string
  note_to_text (int i) /* FIXME: dedup? */
  {
    std::vector<std::string> note_name { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return string_printf ("%s%d", note_name[i % 12].c_str(), i / 12 - 2);
  }
  void
  set_active_notes (const std::vector<int>& notes)
  {
    for (auto& note_led : note_leds)
      {
        bool on = std::find (notes.begin(), notes.end(), note_led.note) != notes.end();
        note_led.led->set_on (on);
      }
  }
  Signal<> signal_toggle_play;
  Signal<> signal_closed;
};

}
