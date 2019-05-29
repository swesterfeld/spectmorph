// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_WINDOW_HH
#define SPECTMORPH_INST_EDIT_WINDOW_HH

#include "sminstrument.hh"
#include "smbuilderthread.hh"
#include "smwindow.hh"

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
class SampleWidget;
class SynthInterface;

class InstEditWindow;
class InstEditParams;
class InstEditBackend
{
  BuilderThread           builder_thread;

  std::mutex              result_mutex;
  std::unique_ptr<WavSet> result_wav_set;
  PlayMode                result_play_mode;
  SynthInterface         *synth_interface;

public:
  InstEditBackend (SynthInterface *synth_interface);

  void switch_to_sample (const Sample *sample, PlayMode play_mode, const Instrument *instrument);
  bool have_builder();
  void on_timer();

  Signal<int, Audio *> signal_have_audio;
};

class InstEditWindow : public Window
{
  Instrument *instrument;
  InstEditBackend m_backend;
  SynthInterface *synth_interface;

  SampleWidget *sample_widget;
  ComboBox *midi_note_combobox = nullptr;

  std::string   note_to_text (int i);
  void          load_sample (const std::string& filename);
  void          on_samples_changed();
  void          on_marker_changed();
  void          update_auto_checkboxes();
  void          on_global_changed();
  Sample::Loop  text_to_loop (const std::string& text);
  std::string   loop_to_text (const Sample::Loop loop);

  ComboBox *sample_combobox;
  ScrollView *sample_scroll_view;
  Label *hzoom_label;
  Label *vzoom_label;
  PlayMode play_mode = PlayMode::SPECTMORPH;
  ComboBox *play_mode_combobox;
  ComboBox *loop_combobox;
  Label *playing_label;
  LineEdit *name_line_edit = nullptr;
  CheckBox *auto_volume_checkbox = nullptr;
  CheckBox *auto_tune_checkbox = nullptr;
  Button   *play_button = nullptr;
  Button   *add_sample_button = nullptr;
  Button   *remove_sample_button = nullptr;
  Label       *progress_label = nullptr;
  ProgressBar *progress_bar = nullptr;
  bool      playing = false;

  InstEditParams *inst_edit_params = nullptr;
  Button         *show_params_button = nullptr;

public:

// morph plan window size
  static const int win_width = 744;
  static const int win_height = 560;

  InstEditWindow (EventLoop& event_loop, Instrument *edit_instrument, SynthInterface *synth_interface, Window *parent_window = nullptr);
  ~InstEditWindow();

  void on_add_sample_clicked();
  void on_remove_sample_clicked();
  void on_update_hzoom (float value);
  void on_update_vzoom (float value);
  void on_show_hide_params();
  void on_save_clicked();
  void on_load_clicked();
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
