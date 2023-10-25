// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_INST_EDIT_WINDOW_HH
#define SPECTMORPH_INST_EDIT_WINDOW_HH

#include "sminstrument.hh"
#include "sminsteditnote.hh"
#include "smbuilderthread.hh"
#include "smwindow.hh"
#include "smloadstereodialog.hh"
#include "sminsteditvolume.hh"

#include <thread>

namespace SpectMorph
{

enum class PlayMode
{
  SPECTMORPH,
  SAMPLE,
  REFERENCE
};

class Label;
class LineEdit;
class ProgressBar;
class CheckBox;
class Button;
class Timer;
class ComboBox;
class Slider;
class SampleWidget;
class SynthInterface;

class IconButton;
class InstEditWindow;
class InstEditParams;
class InstEditBackend
{
  BuilderThread           builder_thread;

  std::mutex              result_mutex;
  bool                    result_updated = false;
  std::unique_ptr<WavSet> result_wav_set;
  std::string             reference;
  SynthInterface         *synth_interface;

  std::unique_ptr<InstEncCache::Group> cache_group;

public:
  InstEditBackend (SynthInterface *synth_interface);

  void update_instrument (const Instrument *instrument, const std::string& reference);
  bool have_builder();
  void on_timer();

  Signal<int, Audio *> signal_have_audio;
};

class InstEditWindow : public Window
{
  Instrument *instrument;
  std::vector<unsigned char> revert_instrument_data;
  InstEditBackend m_backend;
  SynthInterface *synth_interface;
  float         play_gain = 1;

  SampleWidget *sample_widget;
  ComboBox *midi_note_combobox = nullptr;

  std::string   note_to_text (int i);
  void          load_sample (const std::string& filename);
  void          load_sample_convert_from_stereo (const WavData& wav_data, const std::string& filename, LoadStereoDialog::Result result);
  void          on_samples_changed();
  void          on_selected_sample_changed();
  void          on_marker_or_volume_changed();
  void          update_auto_checkboxes();
  void          on_global_changed();
  void          on_reference_changed (const std::string& new_reference);
  void          on_midi_to_reference_changed (bool new_midi_to_reference);
  void          on_gain_changed (float new_gain);
  void          stop_playback();
  Sample::Loop  text_to_loop (const std::string& text);
  std::string   loop_to_text (const Sample::Loop loop);

  ComboBox    *sample_combobox;
  ScrollView  *sample_scroll_view;
  Label       *hzoom_label = nullptr;
  Label       *vzoom_label = nullptr;
  Slider      *hzoom_slider = nullptr;
  PlayMode play_mode = PlayMode::SPECTMORPH;
  ComboBox *play_mode_combobox;
  ComboBox *loop_combobox;
  Label *playing_label;
  Label *time_label = nullptr;
  LineEdit *name_line_edit = nullptr;
  CheckBox *auto_volume_checkbox = nullptr;
  CheckBox *auto_tune_checkbox = nullptr;
  IconButton  *play_button = nullptr;
  Button      *add_sample_button = nullptr;
  Button      *remove_sample_button = nullptr;
  Label       *progress_label = nullptr;
  ProgressBar *progress_bar = nullptr;
  bool      playing = false;
  std::string     reference = "synth-saw.smset";
  bool            midi_to_reference = false;

  InstEditParams *inst_edit_params = nullptr;
  Button         *show_params_button = nullptr;
  Button         *edit_volume_button = nullptr;
  InstEditNote   *inst_edit_note = nullptr;
  InstEditVolume *inst_edit_volume = nullptr;
  Button         *show_pitch_button = nullptr;

public:

// morph plan window size
  static const int win_width = 744;
  static const int win_height = 560;

  InstEditWindow (EventLoop& event_loop, Instrument *edit_instrument, SynthInterface *synth_interface, Window *parent_window = nullptr);
  ~InstEditWindow();

  void clear_edit_instrument();

  void on_clear();
  void on_revert();
  void on_add_sample_clicked();
  void on_remove_sample_clicked();
  void on_update_hzoom (float value);
  void on_update_vzoom (float value);
  void on_drag_scroll (double cx, double dx, double dy);
  void on_show_hide_params();
  void on_show_hide_note();
  void on_show_hide_volume();
  void on_import_clicked();
  void on_export_clicked();
  void on_sample_changed();
  void on_sample_up();
  void on_sample_down();
  void on_midi_note_changed();
  void on_auto_volume_changed (bool new_value);
  void on_auto_tune_changed (bool new_value);
  void on_play_mode_changed();
  void on_loop_changed();
  void on_update_led();
  void on_toggle_play();
  void set_playing (bool new_playing);
  void on_have_audio (int note, Audio *audio);
};

}

#endif
