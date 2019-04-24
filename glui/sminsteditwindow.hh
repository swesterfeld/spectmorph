// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_WINDOW_HH
#define SPECTMORPH_INST_EDIT_WINDOW_HH

#include "sminstrument.hh"
#include "smsamplewidget.hh"
#include "smcombobox.hh"
#include "smtimer.hh"
#include "smwavsetbuilder.hh"
#include "sminsteditparams.hh"
#include "smbutton.hh"
#include "smcheckbox.hh"
#include "smshortcut.hh"

#include <thread>

namespace SpectMorph
{

enum class PlayMode
{
  SPECTMORPH,
  SAMPLE,
  REFERENCE
};

class InstEditWindow;
class InstEditBackend
{
  std::mutex              result_mutex;
  std::unique_ptr<WavSet> result_wav_set;
  PlayMode                result_play_mode;
  SynthInterface         *synth_interface;

  WavSetBuilder          *current_builder = nullptr;
  WavSetBuilder          *next_builder = nullptr;

  double
  note_to_freq (int note)
  {
    return 440 * exp (log (2) * (note - 69) / 12.0);
  }

public:
  InstEditBackend (SynthInterface *synth_interface) :
    synth_interface (synth_interface)
  {
  }
  void switch_to_sample (const Sample *sample, PlayMode play_mode, const Instrument *instrument);

  void
  add_builder (WavSetBuilder *builder, PlayMode play_mode)
  {
    std::lock_guard<std::mutex> lg (result_mutex);

    result_play_mode = play_mode;

    if (current_builder)
      {
        if (next_builder) /* kill and overwrite obsolete next builder */
          delete next_builder;

        next_builder = builder;
      }
    else
      {
        start_as_current (builder);
      }
  }
  void
  start_as_current (WavSetBuilder *builder)
  {
    // must have result_mutex lock
    current_builder = builder;
    new std::thread ([this] () {
      WavSet *wav_set = current_builder->run();

      finish_current_builder (wav_set);
      });
  }
  void
  finish_current_builder (WavSet *wav_set)
  {
    std::lock_guard<std::mutex> lg (result_mutex);

    result_wav_set.reset (wav_set);

    delete current_builder;
    current_builder = nullptr;

    if (next_builder)
      {
        WavSetBuilder *builder = next_builder;

        next_builder = nullptr;
        start_as_current (builder);
      }
  }
  bool
  have_builder()
  {
    std::lock_guard<std::mutex> lg (result_mutex);

    return current_builder != nullptr;
  }
  int current_midi_note() {return 69;}

  Signal<int, Audio *> signal_have_audio;
  void on_timer();
};

class InstEditWindow : public Window
{
  Instrument *instrument;
  InstEditBackend m_backend;
  SynthInterface *synth_interface;

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
      instrument->add_sample (filename);
  }
  void
  on_samples_changed()
  {
    sample_combobox->clear();
    if (instrument->size() == 0)
      {
        sample_combobox->set_text ("");
      }
    for (size_t i = 0; i < instrument->size(); i++)
      {
        Sample *sample = instrument->sample (i);
        std::string text = string_printf ("%s  :  %s", note_to_text (sample->midi_note()).c_str(), sample->short_name.c_str());

        sample_combobox->add_item (text);

        if (int (i) == instrument->selected())
          sample_combobox->set_text (text);
      }
    Sample *sample = instrument->sample (instrument->selected());
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
        m_backend.switch_to_sample (sample, play_mode, instrument);
      }
  }
  void
  on_marker_changed()
  {
    Sample *sample = instrument->sample (instrument->selected());

    if (sample)
      m_backend.switch_to_sample (sample, play_mode, instrument);
  }
  void
  update_auto_checkboxes()
  {
    /* update auto volume checkbox */
    const auto auto_volume = instrument->auto_volume();

    auto_volume_checkbox->set_checked (auto_volume.enabled);

    std::string av_text = "Auto Volume";
    if (auto_volume.enabled)
      {
        switch (auto_volume.method)
        {
          case Instrument::AutoVolume::FROM_LOOP: av_text += " - From Loop";
                                                  break;
          case Instrument::AutoVolume::GLOBAL:    av_text += " - Global";
                                                  break;
        }
      }
    auto_volume_checkbox->set_text (av_text);

    /* update auto tune checkbox */
    const auto auto_tune = instrument->auto_tune();

    auto_tune_checkbox->set_checked (auto_tune.enabled);
    std::string at_text = "Auto Tune";
    if (auto_tune.enabled)
      {
        switch (auto_tune.method)
        {
          case Instrument::AutoTune::SIMPLE:      at_text += " - Simple";
                                                  break;
          case Instrument::AutoTune::ALL_FRAMES:  at_text += " - All Frames";
                                                  break;
          case Instrument::AutoTune::SMOOTH:      at_text += " - Smooth";
                                                  break;
        }
      }
    auto_tune_checkbox->set_text (at_text);
  }
  void
  on_global_changed()
  {
    update_auto_checkboxes();

    Sample *sample = instrument->sample (instrument->selected());

    if (sample)
      m_backend.switch_to_sample (sample, play_mode, instrument);
  }
  ComboBox *sample_combobox;
  ScrollView *sample_scroll_view;
  Label *hzoom_label;
  Label *vzoom_label;
  PlayMode play_mode = PlayMode::SPECTMORPH;
  ComboBox *play_mode_combobox;
  ComboBox *loop_combobox;
  Led *led;
  Label *playing_label;
  CheckBox *auto_volume_checkbox = nullptr;
  CheckBox *auto_tune_checkbox = nullptr;
  Button   *play_button = nullptr;
  bool      playing = false;

  InstEditParams *inst_edit_params = nullptr;
  Button         *show_params_button = nullptr;

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

  InstEditWindow (EventLoop& event_loop, Instrument *edit_instrument, SynthInterface *synth_interface, Window *parent_window = nullptr) :
    Window (event_loop, "SpectMorph - Instrument Editor", win_width, win_height, 0, false, parent_window ? parent_window->native_window() : 0),
    m_backend (synth_interface),
    synth_interface (synth_interface)
  {
    assert (edit_instrument != nullptr);
    instrument = edit_instrument;

    /* attach to model */
    connect (instrument->signal_samples_changed, this, &InstEditWindow::on_samples_changed);
    connect (instrument->signal_marker_changed, this, &InstEditWindow::on_marker_changed);
    connect (instrument->signal_global_changed, this, &InstEditWindow::on_global_changed);

    /* attach to backend */
    connect (m_backend.signal_have_audio, this, &InstEditWindow::on_have_audio);

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

    /*----- sample combobox -----*/
    sample_combobox = new ComboBox (this);
    grid.add_widget (sample_combobox, 1, 5, 91, 3);

    connect (sample_combobox->signal_item_changed, this, &InstEditWindow::on_sample_changed);

    Shortcut *sample_up = new Shortcut (this, PUGL_KEY_UP);
    connect (sample_up->signal_activated, this, &InstEditWindow::on_sample_up);

    Shortcut *sample_down = new Shortcut (this, PUGL_KEY_DOWN);
    connect (sample_down->signal_activated, this, &InstEditWindow::on_sample_down);

    /*----- sample view -----*/
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
    grid.add_widget (play_mode_combobox, 68, 54, 24, 3);
    play_mode_combobox->add_item ("SpectMorph Instrument"); // default
    play_mode_combobox->set_text ("SpectMorph Instrument");
    play_mode_combobox->add_item ("Original Sample");
    play_mode_combobox->add_item ("Reference Instrument");

    /*--- play button ---*/
    play_button = new Button (this, "Play");
    connect (play_button->signal_pressed, this, &InstEditWindow::on_toggle_play);
    grid.add_widget (play_button, 51, 54, 8, 3);

    Shortcut *play_shortcut = new Shortcut (this, ' ');
    connect (play_shortcut->signal_activated, this, &InstEditWindow::on_toggle_play);

    /*--- led ---*/
    led = new Led (this, false);
    grid.add_widget (new Label (this, "Analyzing"), 60, 67, 10, 3);
    grid.add_widget (led, 67, 67.5, 2, 2);

    /*--- playing ---*/
    playing_label = new Label (this, "");
    grid.add_widget (new Label (this, "Playing"), 70, 67, 10, 3);
    grid.add_widget (playing_label, 77, 67, 10, 3);

    /* --- timer --- */
    Timer *timer = new Timer (this);
    connect (timer->signal_timeout, &m_backend, &InstEditBackend::on_timer);
    connect (timer->signal_timeout, this, &InstEditWindow::on_update_led);
    timer->start (0);

    connect (synth_interface->signal_notify_event, [this](SynthNotifyEvent *ne) {
      auto iev = dynamic_cast<InstEditVoice *> (ne);
      if (iev)
        {
          std::vector<float> play_pointers;

          Sample *sample = instrument->sample (instrument->selected());
          if (sample && play_mode != PlayMode::REFERENCE)
            {
              for (size_t i = 0; i < iev->current_pos.size(); i++)
                {
                  if (fabs (iev->fundamental_note[i] - sample->midi_note()) < 0.1)
                    play_pointers.push_back (iev->current_pos[i]);
                }
            }
          sample_widget->set_play_pointers (play_pointers);

          /* this is not 100% accurate if external midi events also affect
           * the state, but it should be good enough */
          bool new_playing = iev->note.size() > 0;
          set_playing (new_playing);

          std::string text = "---";
          if (iev->note.size() > 0)
            text = note_to_text (iev->note[0]);
          playing_label->set_text (text);
        }
    });

    /*--- auto volume ---*/
    auto_volume_checkbox = new CheckBox (this, "Auto Volume");
    connect (auto_volume_checkbox->signal_toggled, this, &InstEditWindow::on_auto_volume_changed);
    grid.add_widget (auto_volume_checkbox, 60, 58.5, 20, 2);

    auto_tune_checkbox = new CheckBox (this, "Auto Tune");
    grid.add_widget (auto_tune_checkbox, 60, 61.5, 20, 2);
    connect (auto_tune_checkbox->signal_toggled, this, &InstEditWindow::on_auto_tune_changed);

    show_params_button = new Button (this, "Show All Parameters...");
    connect (show_params_button->signal_clicked, this, &InstEditWindow::on_show_hide_params);
    grid.add_widget (show_params_button, 60, 64, 25, 3);

    update_auto_checkboxes();

    on_samples_changed();

    // show complete wave
    on_update_hzoom (0);

    on_update_vzoom (0);
  }
  ~InstEditWindow()
  {
    if (inst_edit_params)
      {
        delete inst_edit_params;
        inst_edit_params = nullptr;
      }
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
  on_show_hide_params()
  {
    if (inst_edit_params)
      {
        inst_edit_params->delete_later();
        inst_edit_params = nullptr;
      }
    else
      {
        inst_edit_params = new InstEditParams (this, instrument);
        connect (inst_edit_params->signal_toggle_play, this, &InstEditWindow::on_toggle_play);
        connect (inst_edit_params->signal_closed, [this]() {
          inst_edit_params = nullptr;
        });
      }
  }
  void
  on_save_clicked()
  {
    save_file_dialog ("Select SpectMorph Instrument save filename", "SpectMorph Instrument files", "*.sminst", [=](std::string filename) {
      if (filename != "")
        instrument->save (filename);
    });
  }
  void
  on_load_clicked()
  {
    window()->open_file_dialog ("Select SpectMorph Instrument to load", "SpectMorph Instrument files", "*.sminst", [=](std::string filename) {
      if (filename != "")
        instrument->load (filename);
    });
  }
  void
  on_sample_changed()
  {
    int idx = sample_combobox->current_index();
    if (idx >= 0)
      instrument->set_selected (idx);
  }
  void
  on_sample_up()
  {
    int selected = instrument->selected();

    if (selected > 0)
      instrument->set_selected (selected - 1);
  }
  void
  on_sample_down()
  {
    int selected = instrument->selected();

    if (selected >= 0 && size_t (selected + 1) < instrument->size())
      instrument->set_selected (selected + 1);
  }
  void
  on_midi_note_changed()
  {
    Sample *sample = instrument->sample (instrument->selected());

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
  on_auto_volume_changed (bool new_value)
  {
    Instrument::AutoVolume av = instrument->auto_volume();
    av.enabled = new_value;

    instrument->set_auto_volume (av);
  }
  void
  on_auto_tune_changed (bool new_value)
  {
    Instrument::AutoTune at = instrument->auto_tune();
    at.enabled = new_value;

    instrument->set_auto_tune (at);
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
    Sample *sample = instrument->sample (instrument->selected());

    sample->set_loop (text_to_loop (loop_combobox->text()));
    sample_widget->update_loop();
  }
  void
  on_update_led()
  {
    led->set_on (m_backend.have_builder());
  }
  void
  on_toggle_play()
  {
    Sample *sample = instrument->sample (instrument->selected());
    if (sample)
      synth_interface->synth_inst_edit_note (sample->midi_note(), !playing);
  }
  void
  set_playing (bool new_playing)
  {
    if (playing == new_playing)
      return;

    playing = new_playing;
    play_button->set_text (playing ? "Stop" : "Play");
  }
  void
  on_have_audio (int note, Audio *audio)
  {
    if (!audio)
      return;

    for (size_t i = 0; i < instrument->size(); i++)
      {
        Sample *sample = instrument->sample (i);

        if (sample->midi_note() == note)
          sample->audio.reset (audio->clone());
      }
    sample_widget->update();
  }
};

inline void
InstEditBackend::on_timer()
{
  for (auto ev : synth_interface->get_project()->notify_take_events())
    {
      SynthNotifyEvent *sn_event = SynthNotifyEvent::create (ev);
      if (sn_event)
        {
          synth_interface->signal_notify_event (sn_event);
          delete sn_event;
        }
    }

  std::lock_guard<std::mutex> lg (result_mutex);
  if (result_wav_set)
    {
      printf ("got result!\n");
      for (const auto& wave : result_wav_set->waves)
        signal_have_audio (wave.midi_note, wave.audio);

      if (result_play_mode == PlayMode::SPECTMORPH)
        {
          synth_interface->synth_inst_edit_update (true, result_wav_set.release(), false);
        }
      else if (result_play_mode == PlayMode::SAMPLE)
        {
          synth_interface->synth_inst_edit_update (true, result_wav_set.release(), true);
        }
      else
        {
          Index index;
          index.load_file ("instruments:standard");

          WavSet *wav_set = new WavSet();
          wav_set->load (index.smset_dir() + "/synth-saw.smset");

          synth_interface->synth_inst_edit_update (true, wav_set, false);
        }

      // delete
      result_wav_set.reset();
    }
}

inline void
InstEditBackend::switch_to_sample (const Sample *sample, PlayMode play_mode, const Instrument *instrument)
{
  WavSetBuilder *builder = new WavSetBuilder (instrument, /* keep_samples */ play_mode == PlayMode::SAMPLE);

  add_builder (builder, play_mode);
}

}

#endif
