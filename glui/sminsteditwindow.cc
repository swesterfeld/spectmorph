// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "sminsteditwindow.hh"
#include "sminsteditparams.hh"

#include "smprogressbar.hh"
#include "smmessagebox.hh"
#include "smsamplewidget.hh"
#include "smtimer.hh"
#include "smmenubar.hh"
#include "smslider.hh"
#include "smsimplelines.hh"
#include "smwavsetbuilder.hh"
#include "smwavsetrepo.hh"
#include "smsynthinterface.hh"
#include "smscrollview.hh"
#include "sminstenccache.hh"
#include "smzip.hh"
#include "sminsteditvolume.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

namespace SpectMorph
{

class IconButton : public Button
{
public:
  enum Icon { PLAY, STOP } m_icon;

  IconButton (Widget *parent, Icon icon) :
    Button (parent, ""),
    m_icon (icon)
  {
  }
  void
  set_icon (Icon icon)
  {
    if (icon != m_icon)
      {
        m_icon = icon;
        update();
      }
  }
  void
  draw (const DrawEvent& devent) override
  {
    Button::draw (devent);

    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    double space = 4;
    double size = std::min (width() * 0.55, height() * 0.55) - 2 * space;

    if (m_icon == PLAY)
      {
        const double left = width() / 2 - size / 2 + size * 0.1;
        cairo_move_to (cr, left, height() / 2 - size / 2);
        cairo_line_to (cr, left, height() / 2 + size / 2);
        cairo_line_to (cr, left + size * 0.8, height() / 2);
        //cairo_line_to (cr, width / 2 - size / 2 + size, height / 2);

        cairo_close_path (cr);
        cairo_stroke_preserve (cr);
        cairo_fill (cr);
      }
    else if (m_icon == STOP)
      {
        du.rect_fill (width() / 2 - size / 2, height() / 2 - size / 2, size, size, Color (1, 1, 1));
      }
  }
};

}

// ---------------- InstEditBackend ----------------
//
InstEditBackend::InstEditBackend (SynthInterface *synth_interface) :
  synth_interface (synth_interface),
  cache_group (InstEncCache::the()->create_group())
{
  synth_interface->synth_inst_edit_update (true, nullptr, nullptr);
}

InstEditBackend::~InstEditBackend()
{
  synth_interface->synth_inst_edit_update (false, nullptr, nullptr);
}

void
InstEditBackend::update_instrument (const Instrument *instrument, const string& reference)
{
  WavSetBuilder *builder = new WavSetBuilder (instrument, true);
  builder->set_cache_group (cache_group.get());

  builder_thread.kill_all_jobs();

  std::lock_guard<std::mutex> lg (result_mutex);
  result_updated = true;
  result_wav_set.reset (nullptr);

  this->reference = reference;
  builder_thread.add_job (builder, /* unused: object_id */ 0,
    [this] (WavSet *wav_set)
      {
        std::lock_guard<std::mutex> lg (result_mutex);
        result_updated = true;
        result_wav_set.reset (wav_set);
      }
    );
}

bool
InstEditBackend::have_builder()
{
  return builder_thread.job_count() > 0;
}

void
InstEditBackend::on_timer()
{
  std::lock_guard<std::mutex> lg (result_mutex);
  if (result_updated)
    {
      result_updated = false;
      if (result_wav_set)
        {
          for (const auto& wave : result_wav_set->waves)
            signal_have_audio (wave.midi_note, wave.audio);
        }

      Index index;
      index.load_file ("instruments:standard");

      WavSet *ref_wav_set = WavSetRepo::the()->get (index.smset_dir() + "/" + reference);

      synth_interface->synth_inst_edit_update (true, result_wav_set.release(), ref_wav_set);
    }
}

// ---------------- InstEditWindow ----------------
//
InstEditWindow::InstEditWindow (EventLoop& event_loop, const Instrument *edit_instrument, SynthInterface *synth_interface, Window *parent_window) :
  Window (event_loop, "SpectMorph - Instrument Editor", win_width, win_height, 0, false, parent_window ? parent_window->native_window() : 0),
  m_backend (synth_interface),
  synth_interface (synth_interface)
{
  assert (edit_instrument != nullptr);
  instrument = edit_instrument->clone();

  /* make a backup to be able to revert */
  ZipWriter writer;
  instrument->save (writer);
  revert_instrument_data = writer.data();

  /* attach to model */
  connect (instrument->signal_samples_changed, this, &InstEditWindow::on_samples_changed);
  connect (instrument->signal_selected_sample_changed, this, &InstEditWindow::on_selected_sample_changed);
  connect (instrument->signal_marker_changed, this, &InstEditWindow::on_marker_or_volume_changed);
  connect (instrument->signal_volume_changed, this, &InstEditWindow::on_marker_or_volume_changed);
  connect (instrument->signal_global_changed, this, &InstEditWindow::on_global_changed);

  /* attach to backend */
  connect (m_backend.signal_have_audio, this, &InstEditWindow::on_have_audio);

  FixedGrid grid;

  MenuBar *menu_bar = new MenuBar (this);

  fill_zoom_menu (menu_bar->add_menu ("Zoom"));
  Menu *file_menu = menu_bar->add_menu ("File");

  MenuItem *add_item = file_menu->add_item ("Add Sample...");
  connect (add_item->signal_clicked, this, &InstEditWindow::on_add_sample_clicked);

  MenuItem *import_item = file_menu->add_item ("Import Instrument...");
  connect (import_item->signal_clicked, this, &InstEditWindow::on_import_clicked);

  MenuItem *export_item = file_menu->add_item ("Export Instrument...");
  connect (export_item->signal_clicked, this, &InstEditWindow::on_export_clicked);

  MenuItem *clear_item = file_menu->add_item ("Clear Instrument");
  connect (clear_item->signal_clicked, this, &InstEditWindow::on_clear);

  MenuItem *revert_item = file_menu->add_item ("Revert Instrument");
  connect (revert_item->signal_clicked, this, &InstEditWindow::on_revert);

  grid.add_widget (menu_bar, 1, 1, 91, 3);

  /*----- sample combobox -----*/
  sample_combobox = new ComboBox (this);
  grid.add_widget (sample_combobox, 1, 5, 72, 3);

  connect (sample_combobox->signal_item_changed, this, &InstEditWindow::on_sample_changed);

  Shortcut *sample_up = new Shortcut (this, PUGL_KEY_UP);
  connect (sample_up->signal_activated, this, &InstEditWindow::on_sample_up);

  Shortcut *sample_down = new Shortcut (this, PUGL_KEY_DOWN);
  connect (sample_down->signal_activated, this, &InstEditWindow::on_sample_down);

  /*----- add/remove ----- */
  add_sample_button = new Button (this, "Add...");
  grid.add_widget (add_sample_button, 74, 5, 8, 3);
  connect (add_sample_button->signal_clicked, this, &InstEditWindow::on_add_sample_clicked);

  remove_sample_button = new Button (this, "Remove");
  grid.add_widget (remove_sample_button, 83, 5, 9, 3);
  connect (remove_sample_button->signal_clicked, this, &InstEditWindow::on_remove_sample_clicked);

  /*----- sample view -----*/
  sample_scroll_view = new ScrollView (this);
  grid.add_widget (sample_scroll_view, 1, 8, 91, 46);

  sample_widget = new SampleWidget (sample_scroll_view);
  connect (sample_widget->signal_drag_scroll, this, &InstEditWindow::on_drag_scroll);

  grid.add_widget (sample_widget, 1, 1, 100, 42);
  sample_scroll_view->set_scroll_widget (sample_widget, true, false, /* center_zoom */ true);

  /*----- hzoom -----*/
  grid.add_widget (new Label (this, "HZoom"), 1, 54, 6, 3);
  hzoom_slider = new Slider (this, 0.0);
  grid.add_widget (hzoom_slider, 7, 54, 28, 3);
  connect (hzoom_slider->signal_value_changed, this, &InstEditWindow::on_update_hzoom);

  hzoom_label = new Label (this, "0");
  grid.add_widget (hzoom_label, 36, 54, 10, 3);

  /*----- vzoom -----*/
  vzoom_min_value = log10 (0.25);
  vzoom_max_value = log10 (10);
  double vzoom_def_value = log10 (1);

  grid.add_widget (new Label (this, "VZoom"), 51, 54, 6, 3);
  Slider *vzoom_slider = new Slider (this, (vzoom_def_value - vzoom_min_value) / (vzoom_max_value - vzoom_min_value));
  grid.add_widget (vzoom_slider, 57, 54, 28, 3);
  connect (vzoom_slider->signal_value_changed, this, &InstEditWindow::on_update_vzoom);

  vzoom_label = new Label (this, "0");
  grid.add_widget (vzoom_label, 86, 54, 10, 3);

  /******************* INSTRUMENT *********************/
  grid.dx = 1;
  grid.dy = 60.5;

  /*---- Instrument name ----*/

  name_line_edit = new LineEdit (this, "untitled");
  name_line_edit->set_click_to_focus (true);
  connect (name_line_edit->signal_text_changed, [this] (const string& name) { instrument->set_name (name); });

  grid.add_widget (new Label (this, "Name"), 0, 0, 6, 3);
  grid.add_widget (name_line_edit, 6, 0, 21, 3);

  /*--- Instrument: auto volume ---*/
  auto_volume_checkbox = new CheckBox (this, "Auto Volume");
  connect (auto_volume_checkbox->signal_toggled, this, &InstEditWindow::on_auto_volume_changed);
  grid.add_widget (auto_volume_checkbox, 0, 3.5, 21, 2);

  /*--- Instrument: auto tune ---*/
  auto_tune_checkbox = new CheckBox (this, "Auto Tune");
  grid.add_widget (auto_tune_checkbox, 0, 6.5, 21, 2);
  connect (auto_tune_checkbox->signal_toggled, this, &InstEditWindow::on_auto_tune_changed);

  /*--- Instrument: edit volume / params ---*/
  edit_volume_button = new Button (this, "Volume...");
  grid.add_widget (edit_volume_button, 11, 3, 8, 3);
  connect (edit_volume_button->signal_clicked, this, &InstEditWindow::on_show_hide_volume);

  show_params_button = new Button (this, "Params...");
  connect (show_params_button->signal_clicked, this, &InstEditWindow::on_show_hide_params);
  grid.add_widget (show_params_button, 19, 3, 8, 3);

  /******************* SAMPLE *********************/
  grid.dx = 31;
  grid.dy = 60.5;

  /*---- Sample: midi_note ---- */
  midi_note_combobox = new ComboBox (this);
  connect (midi_note_combobox->signal_item_changed, this, &InstEditWindow::on_midi_note_changed);

  for (int i = 127; i >= 0; i--)
    midi_note_combobox->add_item (note_to_text_verbose (i));

  grid.add_widget (new Label (this, "Midi Note"), 0, 0, 7, 3);
  grid.add_widget (midi_note_combobox, 8, 0, 12, 3);

  /*--- Sample: edit ---*/
  show_pitch_button = new Button (this, "Edit");
  connect (show_pitch_button->signal_clicked, this, &InstEditWindow::on_show_hide_note);
  grid.add_widget (show_pitch_button, 21, 0, 7, 3);

  /*---- Sample: loop mode ----*/

  loop_combobox = new ComboBox (this);
  connect (loop_combobox->signal_item_changed, this, &InstEditWindow::on_loop_changed);

  loop_combobox->add_item (loop_to_text (Sample::Loop::NONE));
  loop_combobox->set_text (loop_to_text (Sample::Loop::NONE));
  loop_combobox->add_item (loop_to_text (Sample::Loop::FORWARD));
  loop_combobox->add_item (loop_to_text (Sample::Loop::PING_PONG));
  loop_combobox->add_item (loop_to_text (Sample::Loop::SINGLE_FRAME));

  grid.add_widget (new Label (this, "Loop"), 0, 3, 7, 3);
  grid.add_widget (loop_combobox, 8, 3, 20, 3);

  /*--- Sample: time --- */
  time_label = new Label (this, "");

  grid.add_widget (new Label (this, "Time"), 0, 6, 10, 3);
  grid.add_widget (time_label, 8, 6, 10, 3);

  /******************* PLAYBACK *********************/
  grid.dx = 62;
  grid.dy = 60.5;

  /*--- Playback: play mode ---*/
  play_mode_combobox = new ComboBox (this);
  connect (play_mode_combobox->signal_item_changed, this, &InstEditWindow::on_play_mode_changed);
  //grid.add_widget (new Label (this, "Play Mode"), 60, 60, 10, 3);
  grid.add_widget (play_mode_combobox, 7.5, 0, 22.5, 3);
  play_mode_combobox->add_item ("SpectMorph Instrument"); // default
  play_mode_combobox->set_text ("SpectMorph Instrument");
  play_mode_combobox->add_item ("Original Sample");
  play_mode_combobox->add_item ("Reference Instrument");

  /*--- Playback: play button ---*/
  play_button = new IconButton (this, IconButton::PLAY);
  connect (play_button->signal_pressed, this, &InstEditWindow::on_toggle_play);
  grid.add_widget (play_button, 0, 0, 6, 6);

  Shortcut *play_shortcut = new Shortcut (this, ' ');
  connect (play_shortcut->signal_activated, this, &InstEditWindow::on_toggle_play);

  /*--- Playback: playing ---*/
  playing_label = new Label (this, "");
  grid.add_widget (new Label (this, "Playing"), 7.5, 3, 10, 3);
  grid.add_widget (playing_label, 13, 3, 10, 3);

  /*--- Playback: auto select */
  auto_select_checkbox = new CheckBox (this, "Auto Select");
  grid.add_widget (auto_select_checkbox, 21, 3.5, 10, 2);
  connect (auto_select_checkbox->signal_toggled, [this] (bool) { signal_auto_select_changed(); });

  /*--- Playback: progress ---*/
  progress_bar = new ProgressBar (this);
  progress_label = new Label (this, "Analyzing");
  grid.add_widget (progress_label, 0, 6, 10, 3);
  grid.add_widget (progress_bar, 7.5, 6.25, 22.5, 2.5);

  /* --- Playback: timer --- */
  Timer *timer = new Timer (this);
  connect (timer->signal_timeout, &m_backend, &InstEditBackend::on_timer);
  connect (timer->signal_timeout, this, &InstEditWindow::on_update_led);
  timer->start (0);

  connect (synth_interface->signal_notify_event, this, &InstEditWindow::on_synth_notify_event);
  // use global coordinates again
  grid.dx = 0;
  grid.dy = 0;

  const double vline1 = 29 + 0.5;
  const double vline2 = 60 + 0.5;
  const double vline3 = 93;

  // need to be below other widgets
  auto hdr_bg = new Widget (this);
  hdr_bg->set_background_color (Color (0.3, 0.3, 0.3));
  grid.add_widget (hdr_bg, 0, 57.25, 93, 2.5);

  auto hdr_inst = new Label (this, "Instrument");
  hdr_inst->set_align (TextAlign::CENTER);
  hdr_inst->set_bold (true);
  grid.add_widget (hdr_inst, 0, 57, vline1, 3);

  auto hdr_sample = new Label (this, "Sample");
  hdr_sample->set_align (TextAlign::CENTER);
  hdr_sample->set_bold (true);
  grid.add_widget (hdr_sample, vline1, 57, vline2 - vline1, 3);

  auto hdr_play = new Label (this, "Playback");
  hdr_play->set_align (TextAlign::CENTER);
  hdr_play->set_bold (true);
  grid.add_widget (hdr_play, vline2, 57, vline3 - vline2, 3);

  grid.add_widget (new VLine (this, Color (0.8, 0.8, 0.8), 2), vline1 - 0.5, 57.25, 1, 12.75);
  grid.add_widget (new VLine (this, Color (0.8, 0.8, 0.8), 2), vline2 - 0.5, 57.25, 1, 12.75);

  on_samples_changed();
  on_global_changed();

  // show complete wave
  on_update_hzoom (0);

  on_update_vzoom (vzoom_slider->value());
}

InstEditWindow::~InstEditWindow()
{
  if (inst_edit_params)
    {
      delete inst_edit_params;
      inst_edit_params = nullptr;
    }
  if (inst_edit_note)
    {
      delete inst_edit_note;
      inst_edit_note = nullptr;
    }
  if (inst_edit_volume)
    {
      delete inst_edit_volume;
      inst_edit_volume = nullptr;
    }

  assert (instrument);
  delete instrument;
}

void
InstEditWindow::load_sample (const string& filename)
{
  if (filename != "")
    {
      /* try loading file */
      WavData wav_data;
      if (wav_data.load (filename))
        {
          if (wav_data.n_channels() == 2) /* stereo: ask user if we should perform stereo -> mono conversion */
            {
              auto dialog = new LoadStereoDialog (this);

              connect (dialog->signal_result, [wav_data, filename, this] (LoadStereoDialog::Result result)
                {
                  /* we captured a copy of the wave data in this lambda */
                  load_sample_convert_from_stereo (wav_data, filename, result);
                });
              dialog->run();
            }
          else if (wav_data.n_channels() == 1)
            {
              instrument->add_sample (wav_data, filename);
            }
          else
            {
              MessageBox::critical (this, "Error",
                                    string_locale_printf ("Loading sample failed:\n'%s'\nUnsupported number of channels (%d).", filename.c_str(), wav_data.n_channels()));
            }
        }
      else
        {
          MessageBox::critical (this, "Error",
                                string_locale_printf ("Loading sample failed:\n'%s'\n%s.", filename.c_str(), wav_data.error_blurb()));
        }
    }
}

void
InstEditWindow::load_sample_convert_from_stereo (const WavData& wav_data, const string& filename, LoadStereoDialog::Result result)
{
  const vector<float>& old_samples = wav_data.samples();
  vector<float> mono_samples;

  switch (result)
    {
      case LoadStereoDialog::Result::LEFT:
        for (size_t i = 0; i < old_samples.size(); i += 2)
          mono_samples.push_back (old_samples[i]);
        break;
      case LoadStereoDialog::Result::RIGHT:
        for (size_t i = 1; i < old_samples.size(); i += 2)
          mono_samples.push_back (old_samples[i]);
        break;
      case LoadStereoDialog::Result::MIX:
        for (size_t i = 0; i < old_samples.size(); i += 2)
          mono_samples.push_back ((old_samples[i] + old_samples[i + 1]) * 0.5f);
        break;
    }

  WavData mono_wav_data (mono_samples, /* n_channels */ 1, wav_data.mix_freq(), wav_data.bit_depth());
  instrument->add_sample (mono_wav_data, filename);
}

void
InstEditWindow::on_synth_notify_event (SynthNotifyEvent *notify_event)
{
  auto iev = dynamic_cast<InstEditVoiceEvent *> (notify_event);
  if (!iev)
    return;

  vector<float> play_pointers;

  Sample *sample = instrument->sample (instrument->selected());
  if (sample)
    {
      for (const auto& voice : iev->voices)
        {
          if (fabs (voice.fundamental_note - sample->midi_note()) < 0.1 &&
              voice.layer < 2) /* no play position pointer for reference */
            {
              double ppos = voice.current_pos;
              if (voice.layer == 0)
                {
                  const double clip_start_ms = sample->get_marker (MARKER_CLIP_START);
                  if (clip_start_ms > 0)
                    ppos += clip_start_ms;
                }
              play_pointers.push_back (ppos);
            }
        }
    }
  sample_widget->set_play_pointers (play_pointers);

  /* this is not 100% accurate if external midi events also affect
   * the state, but it should be good enough */
  bool new_playing = iev->voices.size() > 0;
  set_playing (new_playing);

  string text = "---";
  if (iev->voices.size() > 0)
    text = note_to_text_verbose (iev->voices[0].note);
  playing_label->set_text (text);

  if (inst_edit_note)
    {
      vector<int> active_notes;
      for (const auto& voice : iev->voices)
        active_notes.push_back (voice.note);
      inst_edit_note->set_active_notes (active_notes);
    }

  vector<int> layer0_active_notes;
  for (const auto& voice : iev->voices)
    if (voice.layer == 0)
      {
        int note = lrint (voice.fundamental_note);
        layer0_active_notes.push_back (note);

        if (auto_select_checkbox->checked() && !layer0_playing.count (note))
          {
            for (size_t i = 0; i < instrument->size(); i++)
              if (instrument->sample (i)->midi_note() == note)
                instrument->set_selected (i);
          }
      }
  /* keep track of playing notes on layer 0 for auto select */
  layer0_playing.clear();
  layer0_playing.insert (layer0_active_notes.begin(), layer0_active_notes.end());

  if (inst_edit_volume)
    {
      inst_edit_volume->set_active_notes (layer0_active_notes);
      inst_edit_volume->add_peak (iev->peak);
    }
}

void
InstEditWindow::on_selected_sample_changed()
{
  sample_combobox->clear();
  if (instrument->size() == 0)
    {
      sample_combobox->set_text ("");
    }
  auto used_count = instrument->used_count();
  for (size_t i = 0; i < instrument->size(); i++)
    {
      Sample *sample = instrument->sample (i);
      string text = string_printf ("%s  :  %s", note_to_text_verbose (sample->midi_note()).c_str(), sample->short_name.c_str());

      int c = used_count[sample->midi_note()];
      if (c > 1)
        text += string_printf ("  :  ** error: midi note used %d times **", c);

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
      time_label->set_text ("---");
    }
  else
    {
      midi_note_combobox->set_text (note_to_text_verbose (sample->midi_note()));
      loop_combobox->set_text (loop_to_text (sample->loop()));

      const double time_s = sample->wav_data().samples().size() / sample->wav_data().mix_freq();
      time_label->set_text (string_printf ("%.3f s", time_s));
    }
}

void
InstEditWindow::on_samples_changed()
{
  on_selected_sample_changed();
  m_backend.update_instrument (instrument, reference);
}

void
InstEditWindow::on_marker_or_volume_changed()
{
  m_backend.update_instrument (instrument, reference);
}

void
InstEditWindow::on_reference_changed (const string& new_reference)
{
  if (new_reference != reference)
    {
      reference = new_reference;
      m_backend.update_instrument (instrument, reference);
    }
}

void
InstEditWindow::on_midi_to_reference_changed (bool new_midi_to_reference)
{
  midi_to_reference = new_midi_to_reference;
  synth_interface->synth_inst_edit_midi_to_reference (midi_to_reference);
}

void
InstEditWindow::on_gain_changed (float new_gain)
{
  play_gain = new_gain;
  synth_interface->synth_inst_edit_gain (play_gain);
}

void
InstEditWindow::update_auto_checkboxes()
{
  /* update auto volume checkbox */
  const auto auto_volume = instrument->auto_volume();

  auto_volume_checkbox->set_checked (auto_volume.enabled);

  string av_text = "Auto Volume";
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
  string at_text = "Auto Tune";
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
InstEditWindow::on_global_changed()
{
  update_auto_checkboxes();

  name_line_edit->set_text (instrument->name());
  edit_volume_button->set_visible (!instrument->auto_volume().enabled);

  m_backend.update_instrument (instrument, reference);
}

Sample::Loop
InstEditWindow::text_to_loop (const string& text)
{
  for (int i = 0; ; i++)
    {
      string txt = loop_to_text (Sample::Loop (i));

      if (txt == text)
        return Sample::Loop (i);
      if (txt == "")
        return Sample::Loop (0);
    }
}

string
InstEditWindow::loop_to_text (const Sample::Loop loop)
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

void
InstEditWindow::on_clear()
{
  instrument->clear();
}

void
InstEditWindow::on_revert()
{
  ZipReader reader (revert_instrument_data);
  instrument->load (reader);
}

void
InstEditWindow::on_add_sample_clicked()
{
  FileDialogFormats formats;
  formats.add ("Supported Audio Files", { "wav", "flac", "ogg", "aiff" });
  formats.add ("All Files", { "*" });
  open_file_dialog ("Select Sample to load", formats, [=](string filename) {
    load_sample (filename);
  });
}

void
InstEditWindow::on_remove_sample_clicked()
{
  instrument->remove_sample();
}

void
InstEditWindow::on_drag_scroll (double cx, double dx, double dy)
{
  hzoom_slider->set_value (std::clamp<double> (hzoom_slider->value() - dy * 0.003, 0, 1));
  double factor = pow (2, hzoom_slider->value() * 10);
  sample_scroll_view->change_widget_x_and_width (cx, dx, 89 * factor * 8);
  hzoom_label->set_text (string_printf ("%.1f %%", factor * 100));
}

void
InstEditWindow::on_update_hzoom (float value)
{
  FixedGrid grid;
  double factor = pow (2, value * 10);
  grid.add_widget (sample_widget, 1, 1, 89 * factor, 42);
  sample_scroll_view->on_widget_size_changed();
  hzoom_label->set_text (string_printf ("%.1f %%", factor * 100));
}

void
InstEditWindow::on_update_vzoom (float value)
{
  FixedGrid grid;
  double factor = pow (10, value * (vzoom_max_value - vzoom_min_value) + vzoom_min_value);
  sample_widget->set_vzoom (factor);
  vzoom_label->set_text (string_printf ("%.1f %%", factor * 100));
}

void
InstEditWindow::on_show_hide_params()
{
  if (inst_edit_params)
    {
      inst_edit_params->delete_later();
      inst_edit_params = nullptr;
    }
  else
    {
      inst_edit_params = new InstEditParams (this, instrument, sample_widget);
      connect (inst_edit_params->signal_toggle_play, this, &InstEditWindow::on_toggle_play);
      connect (inst_edit_params->signal_show_volume_editor, this, &InstEditWindow::on_show_hide_volume);
      connect (inst_edit_params->signal_closed, [this]() {
        inst_edit_params = nullptr;
      });
    }
}

void
InstEditWindow::on_show_hide_note()
{
  if (inst_edit_note)
    {
      inst_edit_note->delete_later();
      inst_edit_note = nullptr;
    }
  else
    {
      inst_edit_note = new InstEditNote (this, instrument, synth_interface);
      connect (inst_edit_note->signal_toggle_play, this, &InstEditWindow::on_toggle_play);
      connect (inst_edit_note->signal_closed, [this]() {
        inst_edit_note = nullptr;
      });
    }
}

void
InstEditWindow::on_show_hide_volume()
{
  if (inst_edit_volume)
    {
      inst_edit_volume->delete_later();
      inst_edit_volume = nullptr;
    }
  else
    {
      inst_edit_volume = new InstEditVolume (this, instrument, synth_interface, reference, midi_to_reference, play_gain, this);
      connect (inst_edit_volume->signal_toggle_play, this, &InstEditWindow::on_toggle_play);
      connect (inst_edit_volume->signal_closed, [this]() {
        inst_edit_volume = nullptr;
      });
      connect (inst_edit_volume->signal_reference_changed, this, &InstEditWindow::on_reference_changed);
      connect (inst_edit_volume->signal_midi_to_reference_changed, this, &InstEditWindow::on_midi_to_reference_changed);
      connect (inst_edit_volume->signal_gain_changed, this, &InstEditWindow::on_gain_changed);
    }
}

void
InstEditWindow::on_export_clicked()
{
  FileDialogFormats formats ("SpectMorph Instrument files", "sminst");
  save_file_dialog ("Select SpectMorph Instrument export filename", formats, [=](string filename) {
    if (filename != "")
      {
        ZipWriter zip_writer (filename);

        Error error = instrument->save (zip_writer);
        if (error)
          {
            MessageBox::critical (this, "Error",
                                  string_locale_printf ("Exporting instrument failed:\n'%s'\n%s.", filename.c_str(), error.message()));
          }
      }
  });
}

void
InstEditWindow::on_import_clicked()
{
  FileDialogFormats formats ("SpectMorph Instrument files", "sminst");
  window()->open_file_dialog ("Select SpectMorph Instrument to import", formats, [=](string filename) {
    if (filename != "")
      {
        Error error = instrument->load (filename);
        if (error)
          {
            MessageBox::critical (this, "Error",
                                  string_locale_printf ("Importing instrument failed:\n'%s'\n%s.", filename.c_str(), error.message()));
          }
      }
  });
}

void
InstEditWindow::on_sample_changed()
{
  int idx = sample_combobox->current_index();
  if (idx >= 0)
    {
      stop_playback();
      instrument->set_selected (idx);
    }
}

void
InstEditWindow::on_sample_up()
{
  int selected = instrument->selected();

  if (selected > 0)
    {
      stop_playback();
      instrument->set_selected (selected - 1);
    }
}

void
InstEditWindow::on_sample_down()
{
  int selected = instrument->selected();

  if (selected >= 0 && size_t (selected + 1) < instrument->size())
    {
      stop_playback();
      instrument->set_selected (selected + 1);
    }
}

void
InstEditWindow::stop_playback()
{
  if (playing)
    on_toggle_play();
}

void
InstEditWindow::on_midi_note_changed()
{
  Sample *sample = instrument->sample (instrument->selected());

  if (!sample)
    return;
  for (int i = 0; i < 128; i++)
    {
      if (midi_note_combobox->text() == note_to_text_verbose (i))
        {
          sample->set_midi_note (i);
        }
    }
}

void
InstEditWindow::on_auto_volume_changed (bool new_value)
{
  Instrument::AutoVolume av = instrument->auto_volume();
  av.enabled = new_value;

  instrument->set_auto_volume (av);
}

void
InstEditWindow::on_auto_tune_changed (bool new_value)
{
  Instrument::AutoTune at = instrument->auto_tune();
  at.enabled = new_value;

  instrument->set_auto_tune (at);
}

void
InstEditWindow::on_play_mode_changed()
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
InstEditWindow::on_loop_changed()
{
  Sample *sample = instrument->sample (instrument->selected());

  sample->set_loop (text_to_loop (loop_combobox->text()));
  sample_widget->update_loop();
}

void
InstEditWindow::on_update_led()
{
  bool analyzing = m_backend.have_builder();
  if (m_backend.have_builder())
    {
      progress_label->set_text ("Analyzing");
      progress_bar->set_value (-1.0);
    }
  else
    {
      progress_label->set_text ("Ready.");
      progress_bar->set_value (1.0);
    }
  if (inst_edit_volume)
    inst_edit_volume->set_analyzing (analyzing);
}

void
InstEditWindow::on_toggle_play()
{
  Sample *sample = instrument->sample (instrument->selected());
  if (sample)
    {
      uint layer = 0;
      if (play_mode == PlayMode::SAMPLE)
        layer = 1;
      if (play_mode == PlayMode::REFERENCE)
        layer = 2;

      synth_interface->synth_inst_edit_note (sample->midi_note(), !playing, layer);
    }
}

void
InstEditWindow::set_playing (bool new_playing)
{
  if (playing == new_playing)
    return;

  playing = new_playing;
  play_button->set_icon (playing ? IconButton::STOP : IconButton::PLAY);
}

/*
 * Instrument ownership:
 *   - in our constructor we make a deep copy of the instrument we want to edit
 *     so we can modify it and use its signals without any interference
 *   - in this function we return a deep copy of the edited instruments to
 *     ensure that the caller can modify, keep or destroy the Instrument at any
 *     time
 *   - in our destructor, we delete our copy of the instrument
 *
 * This ensures that draw events or timer events that happen after the
 * window has been closed (but before the InstEditWindow has been destroyed)
 * can always access our own private version of the Instrument.
 */
std::unique_ptr<Instrument>
InstEditWindow::get_modified_instrument()
{
  return std::unique_ptr<Instrument> (instrument->clone());
}

void
InstEditWindow::on_have_audio (int note, Audio *audio)
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
  if (inst_edit_volume)
    inst_edit_volume->audio_updated();
}

void
InstEditWindow::set_auto_select (bool auto_select)
{
  if (auto_select != auto_select_checkbox->checked())
    {
      auto_select_checkbox->set_checked (auto_select);
      signal_auto_select_changed();
    }
}

bool
InstEditWindow::auto_select() const
{
  return auto_select_checkbox->checked();
}
