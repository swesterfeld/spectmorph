// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_WINDOW_HH
#define SPECTMORPH_INST_EDIT_WINDOW_HH

#include "sminstrument.hh"

namespace SpectMorph
{

enum class PlayMode
{
  SAMPLE,
  SPECTMORPH,
  REFERENCE
};

class Backend
{
public:
  virtual void switch_to_sample (const Sample *sample, PlayMode play_mode, const Instrument *instrument = nullptr) = 0;
  virtual bool have_builder() = 0;
  virtual int current_midi_note() = 0;
};

class InstEditWindow : public Window
{
  Instrument instrument;

  SampleWidget *sample_widget;
  ComboBox *midi_note_combobox = nullptr;

  std::string
  note_to_text (int i)
  {
    std::vector<std::string> note_name { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return string_printf ("%d  :  %s%d", i, note_name[i % 12].c_str(), i / 12 - 2);
  }
  void
  load_sample (const std::string& filename)
  {
    if (filename != "")
      instrument.add_sample (filename);
  }
  void
  on_samples_changed()
  {
    sample_combobox->clear();
    if (instrument.size() == 0)
      {
        sample_combobox->set_text ("");
      }
    for (size_t i = 0; i < instrument.size(); i++)
      {
        Sample *sample = instrument.sample (i);
        std::string text = string_printf ("%s  :  %s", note_to_text (sample->midi_note()).c_str(), sample->filename.c_str());

        sample_combobox->add_item (text);

        if (int (i) == instrument.selected())
          sample_combobox->set_text (text);
      }
    Sample *sample = instrument.sample (instrument.selected());
    sample_widget->set_sample (sample);
    midi_note_combobox->set_enabled (sample != nullptr);
    sample_combobox->set_enabled (sample != nullptr);
    play_mode_combobox->set_enabled (sample != nullptr);
    loop_combobox->set_enabled (sample != nullptr);
    if (!sample)
      {
        midi_note_combobox->set_text ("");
        loop_combobox->set_text ("");
      }
    else
      {
        midi_note_combobox->set_text (note_to_text (sample->midi_note()));
        loop_combobox->set_text (loop_to_text (sample->loop()));
      }
    if (sample)
      {
        backend->switch_to_sample (sample, play_mode, &instrument);
      }
  }
  void
  on_marker_changed()
  {
    Sample *sample = instrument.sample (instrument.selected());

    if (sample)
      {
        sample_widget->update_markers();
        backend->switch_to_sample (sample, play_mode, &instrument);
      }
  }
  ComboBox *sample_combobox;
  ScrollView *sample_scroll_view;
  Label *hzoom_label;
  Label *vzoom_label;
  Backend *backend;
  PlayMode play_mode = PlayMode::SAMPLE;
  ComboBox *play_mode_combobox;
  ComboBox *loop_combobox;
  Led *led;
  Label *playing_label;

  Sample::Loop
  text_to_loop (const std::string& text)
  {
    for (int i = 0; ; i++)
      {
        std::string txt = loop_to_text (Sample::Loop (i));

        if (txt == text)
          return Sample::Loop (i);
        if (txt == "")
          return Sample::Loop (0);
      }
  }
  std::string
  loop_to_text (const Sample::Loop loop)
  {
    switch (loop)
      {
        case Sample::Loop::NONE:        return "None";
        case Sample::Loop::FORWARD:     return "Forward";
        case Sample::Loop::PING_PONG:   return "Ping Pong";
        case Sample::Loop::SINGLE_FRAME:return "Single Frame";
      }
    return ""; /* not found */
  }
public:
  void
  on_add_sample_clicked()
  {
    open_file_dialog ("Select Sample to load", "Wav Files", "*.wav", [=](std::string filename) {
      load_sample (filename);
    });
  }
// morph plan window size
  static const int win_width = 744;
  static const int win_height = 560;

  InstEditWindow (const std::string& test_sample, Backend *backend) :
    Window ("SpectMorph - Instrument Editor", win_width, win_height),
    backend (backend)
  {
    /* attach to model */
    connect (instrument.signal_samples_changed, this, &InstEditWindow::on_samples_changed);
    connect (instrument.signal_marker_changed, this, &InstEditWindow::on_marker_changed);

    FixedGrid grid;

    MenuBar *menu_bar = new MenuBar (this);

    fill_zoom_menu (menu_bar->add_menu ("Zoom"));
    Menu *file_menu = menu_bar->add_menu ("File");

    MenuItem *add_item = file_menu->add_item ("Add Sample...");
    connect (add_item->signal_clicked, this, &InstEditWindow::on_add_sample_clicked);

    MenuItem *load_item = file_menu->add_item ("Load Instrument...");
    connect (load_item->signal_clicked, this, &InstEditWindow::on_load_clicked);

    MenuItem *save_item = file_menu->add_item ("Save Instrument...");
    connect (save_item->signal_clicked, this, &InstEditWindow::on_save_clicked);

    grid.add_widget (menu_bar, 1, 1, 91, 3);

    sample_combobox = new ComboBox (this);
    grid.add_widget (sample_combobox, 1, 5, 91, 3);

    connect (sample_combobox->signal_item_changed, this, &InstEditWindow::on_sample_changed);

    sample_scroll_view = new ScrollView (this);
    grid.add_widget (sample_scroll_view, 1, 8, 91, 46);

    sample_widget = new SampleWidget (sample_scroll_view);

    grid.add_widget (sample_widget, 1, 1, 100, 42);
    sample_scroll_view->set_scroll_widget (sample_widget, true, false, /* center_zoom */ true);

    /*----- hzoom -----*/
    grid.add_widget (new Label (this, "HZoom"), 1, 54, 10, 3);
    Slider *hzoom_slider = new Slider (this, 0.0);
    grid.add_widget (hzoom_slider, 8, 54, 30, 3);
    connect (hzoom_slider->signal_value_changed, this, &InstEditWindow::on_update_hzoom);

    hzoom_label = new Label (this, "0");
    grid.add_widget (hzoom_label, 40, 54, 10, 3);

    /*----- vzoom -----*/
    grid.add_widget (new Label (this, "VZoom"), 1, 57, 10, 3);
    Slider *vzoom_slider = new Slider (this, 0.0);
    grid.add_widget (vzoom_slider, 8, 57, 30, 3);
    connect (vzoom_slider->signal_value_changed, this, &InstEditWindow::on_update_vzoom);

    vzoom_label = new Label (this, "0");
    grid.add_widget (vzoom_label, 40, 57, 10, 3);

    /*---- midi_note ---- */
    midi_note_combobox = new ComboBox (this);
    connect (midi_note_combobox->signal_item_changed, this, &InstEditWindow::on_midi_note_changed);

    for (int i = 127; i >= 0; i--)
      midi_note_combobox->add_item (note_to_text (i));

    grid.add_widget (new Label (this, "Midi Note"), 1, 60, 10, 3);
    grid.add_widget (midi_note_combobox, 8, 60, 20, 3);

    /*---- loop mode ----*/

    loop_combobox = new ComboBox (this);
    connect (loop_combobox->signal_item_changed, this, &InstEditWindow::on_loop_changed);

    loop_combobox->add_item (loop_to_text (Sample::Loop::NONE));
    loop_combobox->set_text (loop_to_text (Sample::Loop::NONE));
    loop_combobox->add_item (loop_to_text (Sample::Loop::FORWARD));
    loop_combobox->add_item (loop_to_text (Sample::Loop::PING_PONG));
    loop_combobox->add_item (loop_to_text (Sample::Loop::SINGLE_FRAME));

    grid.add_widget (new Label (this, "Loop"), 1, 63, 10, 3);
    grid.add_widget (loop_combobox, 8, 63, 20, 3);

    /*--- play mode ---*/
    play_mode_combobox = new ComboBox (this);
    connect (play_mode_combobox->signal_item_changed, this, &InstEditWindow::on_play_mode_changed);
    grid.add_widget (new Label (this, "Play Mode"), 60, 54, 10, 3);
    grid.add_widget (play_mode_combobox, 68, 54, 20, 3);
    play_mode_combobox->add_item ("Original Sample");
    play_mode_combobox->set_text ("Original Sample"); // default
    play_mode_combobox->add_item ("SpectMorph Instrument");
    play_mode_combobox->add_item ("Reference Instrument");

    /*--- led ---*/
    led = new Led (this, false);
    grid.add_widget (new Label (this, "Analyzing"), 70, 64, 10, 3);
    grid.add_widget (led, 77, 64.5, 2, 2);

    /*--- playing ---*/
    playing_label = new Label (this, "");
    grid.add_widget (new Label (this, "Playing"), 70, 67, 10, 3);
    grid.add_widget (playing_label, 77, 67, 10, 3);

    instrument.load (test_sample);

    // show complete wave
    on_update_hzoom (0);

    on_update_vzoom (0);
  }
  void
  on_update_hzoom (float value)
  {
    FixedGrid grid;
    double factor = pow (2, value * 10);
    grid.add_widget (sample_widget, 1, 1, 89 * factor, 42);
    sample_scroll_view->on_widget_size_changed();
    hzoom_label->set_text (string_printf ("%.1f %%", factor * 100));
  }
  void
  on_update_vzoom (float value)
  {
    FixedGrid grid;
    double factor = pow (10, value);
    sample_widget->set_vzoom (factor);
    vzoom_label->set_text (string_printf ("%.1f %%", factor * 100));
  }
  void
  on_save_clicked()
  {
    instrument.save ("/tmp/x.sminst");
  }
  void
  on_load_clicked()
  {
    instrument.load ("/tmp/x.sminst");
  }
  void
  on_sample_changed()
  {
    int idx = sample_combobox->current_index();
    if (idx >= 0)
      instrument.set_selected (idx);
  }
  void
  on_midi_note_changed()
  {
    Sample *sample = instrument.sample (instrument.selected());

    if (!sample)
      return;
    for (int i = 0; i < 128; i++)
      {
        if (midi_note_combobox->text() == note_to_text (i))
          {
            sample->set_midi_note (i);
          }
      }
  }
  void
  on_play_mode_changed()
  {
    int idx = play_mode_combobox->current_index();
    if (idx >= 0)
      {
        play_mode = static_cast <PlayMode> (idx);

        // this may do a little more than we need, but it updates play_mode
        // in the backend
        on_samples_changed();
      }
  }
  void
  on_loop_changed()
  {
    Sample *sample = instrument.sample (instrument.selected());

    sample->set_loop (text_to_loop (loop_combobox->text()));
  }
  void
  update_led()
  {
    led->set_on (backend->have_builder());

    int note = backend->current_midi_note();
    playing_label->set_text (note >= 0 ? note_to_text (note) : "---");
  }
};

}

#endif
