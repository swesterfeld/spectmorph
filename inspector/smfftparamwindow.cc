/*
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smfftparamwindow.hh"
#include "smmath.hh"
#include <assert.h>
#include <birnet/birnet.hh>

#include <QGridLayout>
#include <QGroupBox>

using namespace SpectMorph;

using std::string;

#define TEXT_FFT "Fourier Transform"
#define TEXT_CWT "Wavelet Transform"
#define TEXT_LPC "LPC Transform"

#define TEXT_VTIME "Frequency dependant time scale"
#define TEXT_CTIME "Constant time scale"

FFTParamWindow::FFTParamWindow()
{
  setWindowTitle ("Transform Parameters");
  resize (500, 200);

  QLabel *transform_label = new QLabel ("Transform Type");
  transform_combobox = new QComboBox();
  transform_combobox->addItem (TEXT_FFT);
  transform_combobox->addItem (TEXT_CWT);
  transform_combobox->addItem (TEXT_LPC);
  connect (transform_combobox, SIGNAL (currentIndexChanged (int)), this, SLOT (on_value_changed()));

  QGroupBox *fft_groupbox = new QGroupBox ("Fourier Transform Parameters");
  QGridLayout *fft_grid = new QGridLayout();

  // FFT Frame Size
  fft_grid->addWidget (new QLabel ("FFT Frame Size"), 0, 0);
  fft_frame_size_slider = new QSlider (Qt::Horizontal);
  fft_frame_size_slider->setRange (-1000, 1000);
  fft_frame_size_label = new QLabel();
  fft_grid->addWidget (fft_frame_size_slider, 0, 1);
  fft_grid->addWidget (fft_frame_size_label, 0, 2);
  connect (fft_frame_size_slider, SIGNAL (valueChanged (int)), this, SLOT (on_value_changed()));

  // FFT Frame Overlap
  fft_grid->addWidget (new QLabel ("FFT Frame Overlap"), 1, 0);
  fft_frame_overlap_slider = new QSlider (Qt::Horizontal);
  fft_frame_overlap_slider->setRange (-1000, 2000);
  fft_frame_overlap_label = new QLabel();
  fft_grid->addWidget (fft_frame_overlap_slider, 1, 1);
  fft_grid->addWidget (fft_frame_overlap_label, 1, 2);
  connect (fft_frame_overlap_slider, SIGNAL (valueChanged (int)), this, SLOT (on_value_changed()));

  fft_groupbox->setLayout (fft_grid);

  QGroupBox *cwt_groupbox = new QGroupBox ("Wavelet Transform Parameters");
  QGridLayout *cwt_grid = new QGridLayout();

  // CWT Mode
  cwt_mode_combobox = new QComboBox();
  cwt_mode_combobox->addItem (TEXT_VTIME);
  cwt_mode_combobox->addItem (TEXT_CTIME);
  cwt_grid->addWidget (new QLabel ("Transform Mode"), 0, 0);
  cwt_grid->addWidget (cwt_mode_combobox, 0, 1, 1, 2);
  connect (cwt_mode_combobox, SIGNAL (currentIndexChanged (int)), this, SLOT (on_value_changed()));

  // CWT Frequency Resolution
  cwt_grid->addWidget (new QLabel ("Frequency Resolution"), 1, 0);
  cwt_freq_res_slider = new QSlider (Qt::Horizontal);
  cwt_freq_res_slider->setRange (1000, 100000);
  cwt_freq_res_slider->setValue (25000);
  cwt_freq_res_label = new QLabel();
  cwt_grid->addWidget (cwt_freq_res_slider, 1, 1);
  cwt_grid->addWidget (cwt_freq_res_label, 1, 2);
  connect (cwt_freq_res_slider, SIGNAL (valueChanged (int)), this, SLOT (on_value_changed()));

  // CWT Time Resolution
  cwt_grid->addWidget (new QLabel ("Time Resolution"), 2, 0);
  cwt_time_res_slider = new QSlider (Qt::Horizontal);
  cwt_time_res_slider->setRange (-1000, 1000);
  cwt_time_res_label = new QLabel();
  cwt_grid->addWidget (cwt_time_res_slider, 2, 1);
  cwt_grid->addWidget (cwt_time_res_label, 2, 2);
  connect (cwt_time_res_slider, SIGNAL (valueChanged (int)), this, SLOT (on_value_changed()));

  cwt_groupbox->setLayout (cwt_grid);

  // Progress bar
  progress_bar = new QProgressBar();
  progress_bar->setRange (0, 1000);

  QGridLayout *grid = new QGridLayout();
  grid->addWidget (transform_label, 0, 0);
  grid->addWidget (transform_combobox, 0, 1);
  grid->addWidget (fft_groupbox, 1, 0, 1, 2);
  grid->addWidget (cwt_groupbox, 2, 0, 1, 2);
  grid->addWidget (progress_bar, 3, 0, 1, 2);
  setLayout (grid);

  on_value_changed(); // init value labels
}

AnalysisParams
FFTParamWindow::get_analysis_params()
{
  AnalysisParams params;

  string transform_text = transform_combobox->currentText().toLatin1().data();

  if (transform_text == TEXT_FFT)
    params.transform_type = SM_TRANSFORM_FFT;
  else if (transform_text == TEXT_CWT)
    params.transform_type = SM_TRANSFORM_CWT;
  else if (transform_text == TEXT_LPC)
    params.transform_type = SM_TRANSFORM_LPC;
  else
    assert (false);

  string cwt_mode_text = cwt_mode_combobox->currentText().toLatin1().data();
  if (cwt_mode_text == TEXT_VTIME)
    params.cwt_mode = SM_CWT_MODE_VTIME;
  else if (cwt_mode_text == TEXT_CTIME)
    params.cwt_mode = SM_CWT_MODE_CTIME;
  else
    assert (false);

  params.frame_size_ms = get_frame_size();
  params.frame_step_ms = get_frame_step();

  params.cwt_freq_resolution = cwt_freq_res_slider->value() / 1000.0;
  params.cwt_time_resolution = get_cwt_time_resolution();

  return params;
}


double
FFTParamWindow::get_frame_size()
{
  return 40 * pow (10, fft_frame_size_slider->value() / 1000.0);
}

double
FFTParamWindow::get_cwt_time_resolution()
{
  return 40 * pow (10, cwt_time_res_slider->value() / 1000.0);
}

double
FFTParamWindow::get_frame_step()
{
  return get_frame_size() / get_frame_overlap();
}

double
FFTParamWindow::get_frame_overlap()
{
  return pow (4, fft_frame_overlap_slider->value() / 1000.0 + 1);
}

void
FFTParamWindow::on_value_changed()
{
  fft_frame_size_label->setText (Birnet::string_printf ("%.1f ms", get_frame_size()).c_str());
  fft_frame_overlap_label->setText (Birnet::string_printf ("%.1f", get_frame_overlap()).c_str());
  cwt_freq_res_label->setText (Birnet::string_printf ("%.1f Hz", cwt_freq_res_slider->value() / 1000.0).c_str());
  cwt_time_res_label->setText (Birnet::string_printf ("%.1f ms", get_cwt_time_resolution()).c_str());

  emit params_changed();
}

void
FFTParamWindow::set_progress (double progress)
{
  progress_bar->setValue (sm_round_positive (progress * 1000));
}
