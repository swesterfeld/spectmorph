// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_NAVIGATOR_HH
#define SPECTMORPH_NAVIGATOR_HH

#include "smwavset.hh"
#include "smtimefreqwindow.hh"
#include "smplayerwindow.hh"
#include "smlpcwindow.hh"
#include "smdisplayparamwindow.hh"
#include "smfftparamwindow.hh"
#include "smspectrumwindow.hh"
#include "smsamplewindow.hh"

#include <QComboBox>
#include <QTreeView>
#include <QPushButton>

#include <memory>

namespace SpectMorph {

class TreeModel;
class Navigator : public QWidget
{
  Q_OBJECT

private:
  std::string           smset_dir;
  QComboBox            *smset_combobox;
  WavSet                wset;
  std::string           wset_filename;
  std::string           wset_active_text;
  bool                  wset_edit;
  std::vector<float>    decoded_samples;

  std::unique_ptr<WavData> wav_data;

  Audio                *audio;
  TreeModel            *tree_model;
  QTreeView            *tree_view;
  QPushButton          *source_button;
  QPushButton          *show_position_button;
  QPushButton          *show_analysis_button;
  QPushButton          *show_frequency_grid_button;

  SampleWindow         *sample_window;
  PlayerWindow         *player_window;
  TimeFreqWindow       *time_freq_window;
  FFTParamWindow       *m_fft_param_window;
  SpectrumWindow       *spectrum_window;
  DisplayParamWindow   *m_display_param_window;
  LPCWindow            *lpc_window;

public:
  Navigator (const std::string& filename);

  GslDataHandle        *get_dhandle();
  const WavData        *get_wav_data();
  Audio                *get_audio();
  bool                  get_show_position();
  bool                  get_show_analysis();
  bool                  get_show_frequency_grid();
  bool                  spectmorph_signal_active();
  FFTParamWindow       *fft_param_window();
  DisplayParamWindow   *display_param_window();
  bool                  handle_close_event(); /* returns true if close is ok */
  std::string           title();

public slots:
  void on_combo_changed();
  void on_view_sample();
  void on_view_player();
  void on_selection_changed();
  void on_view_time_freq();
  void on_view_fft_params();
  void on_show_position_changed();
  void on_show_analysis_changed();
  void on_show_frequency_grid_changed();
  void on_view_spectrum();
  void on_view_display_params();
  void on_view_lpc();
  void on_next_sample();
  void on_prev_sample();
  void on_audio_edit();
  void on_save_clicked();

signals:
  void dhandle_changed();
  void wav_data_changed();
  void show_position_changed();
  void show_analysis_changed();
  void show_frequency_grid_changed();
  void title_changed();
};

}

#endif
