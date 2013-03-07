// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_FFTPARAMWINDOW_HH
#define SPECTMORPH_FFTPARAMWINDOW_HH

#include "smspectrumview.hh"
#include "smtimefreqview.hh"
#include "smzoomcontroller.hh"

#include <QComboBox>
#include <QProgressBar>

namespace SpectMorph {

class FFTParamWindow : public QWidget
{
  Q_OBJECT

  QComboBox *transform_combobox;
  QSlider   *fft_frame_size_slider;
  QLabel    *fft_frame_size_label;

  QSlider   *fft_frame_overlap_slider;
  QLabel    *fft_frame_overlap_label;

  QComboBox *cwt_mode_combobox;

  QSlider   *cwt_freq_res_slider;
  QLabel    *cwt_freq_res_label;

  QSlider   *cwt_time_res_slider;
  QLabel    *cwt_time_res_label;

  QProgressBar *progress_bar;

public:
  FFTParamWindow();

  AnalysisParams  get_analysis_params();
  double          get_frame_overlap();
  double          get_frame_size();
  double          get_frame_step();
  double          get_cwt_time_resolution();
  void            set_progress (double progress);

public slots:
  void            on_value_changed();

signals:
  void            params_changed();
};

}

#endif
