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

  // CWT Frequency Resolution
  cwt_grid->addWidget (new QLabel ("Frequency Resolution"), 0, 0);
  cwt_freq_res_slider = new QSlider (Qt::Horizontal);
  cwt_freq_res_slider->setRange (1000, 100000);
  cwt_freq_res_slider->setValue (25000);
  cwt_freq_res_label = new QLabel();
  cwt_grid->addWidget (cwt_freq_res_slider, 0, 1);
  cwt_grid->addWidget (cwt_freq_res_label, 0, 2);
  connect (cwt_freq_res_slider, SIGNAL (valueChanged (int)), this, SLOT (on_value_changed()));

  // CWT Time Resolution
  cwt_grid->addWidget (new QLabel ("Time Resolution"), 1, 0);
  cwt_time_res_slider = new QSlider (Qt::Horizontal);
  cwt_time_res_slider->setRange (-1000, 1000);
  cwt_time_res_label = new QLabel();
  cwt_grid->addWidget (cwt_time_res_slider, 1, 1);
  cwt_grid->addWidget (cwt_time_res_label, 1, 2);
  connect (cwt_time_res_slider, SIGNAL (valueChanged (int)), this, SLOT (on_value_changed()));

  cwt_groupbox->setLayout (cwt_grid);

  QGridLayout *grid = new QGridLayout();
  grid->addWidget (transform_label, 0, 0);
  grid->addWidget (transform_combobox, 0, 1);
  grid->addWidget (fft_groupbox, 1, 0, 1, 2);
  grid->addWidget (cwt_groupbox, 2, 0, 1, 2);
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

#if 0
  if (cwt_mode_combobox.get_active_text() == TEXT_VTIME)
    params.cwt_mode = SM_CWT_MODE_VTIME;
  else if (cwt_mode_combobox.get_active_text() == TEXT_CTIME)
    params.cwt_mode = SM_CWT_MODE_CTIME;
  else
    assert (false);
#endif
  params.cwt_mode = SM_CWT_MODE_CTIME;

  params.frame_size_ms = get_frame_size();
  params.frame_step_ms = get_frame_step();

  params.cwt_freq_resolution = cwt_freq_res_slider->value() / 1000.0;
  params.cwt_time_resolution = get_cwt_time_resolution();

  return params;
}


#if 0
FFTParamWindow::FFTParamWindow() :
  table (5, 3),
  frame_size_scale (-1, 1, 0.01),
  frame_overlap_scale (-1, 2, 0.01),
  cwt_freq_res_scale (1, 100, 0.01),
  cwt_time_res_scale (-1, 1, 0.01)
{
  set_border_width (10);
  set_default_size (500, 200);
  set_title ("Transform Parameters");

  table.attach (transform_label, 0, 1, 0, 1, Gtk::SHRINK);
  table.attach (transform_combobox, 1, 3, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND);
  transform_label.set_text ("Transform Type");
  transform_combobox.append_text (TEXT_FFT);
  transform_combobox.append_text (TEXT_CWT);
  transform_combobox.append_text (TEXT_LPC);
  transform_combobox.set_active_text (TEXT_FFT);
  transform_combobox.signal_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  fft_frame_table.attach (frame_size_label, 0, 1, 0, 1, Gtk::SHRINK);
  fft_frame_table.attach (frame_size_scale, 1, 2, 0, 1);
  fft_frame_table.attach (frame_size_value_label, 2, 3, 0, 1, Gtk::SHRINK);
  frame_size_label.set_text ("FFT Frame Size");
  frame_size_scale.set_draw_value (false);
  frame_size_scale.signal_value_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  fft_frame_table.attach (frame_overlap_label, 0, 1, 1, 2, Gtk::SHRINK);
  fft_frame_table.attach (frame_overlap_scale, 1, 2, 1, 2);
  fft_frame_table.attach (frame_overlap_value_label, 2, 3, 1, 2, Gtk::SHRINK);
  frame_overlap_label.set_text ("FFT Frame Overlap");
  frame_overlap_scale.set_draw_value (false);
  frame_overlap_scale.signal_value_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  fft_frame.set_label ("Fourier Transform Parameters");
  fft_frame_table.set_border_width (10);
  fft_frame.add (fft_frame_table);
  table.attach (fft_frame, 0, 3, 1, 2);

  cwt_frame.set_label ("Wavelet Transform Parameters");
  cwt_frame.add (cwt_frame_table);
  cwt_frame_table.set_border_width (10);
  table.attach (cwt_frame, 0, 3, 2, 3);

  cwt_mode_label.set_label ("Transform Mode");
  cwt_mode_combobox.append_text (TEXT_VTIME);
  cwt_mode_combobox.append_text (TEXT_CTIME);
  cwt_mode_combobox.set_active_text (TEXT_VTIME);
  cwt_frame_table.attach (cwt_mode_label, 0, 1, 0, 1, Gtk::SHRINK);
  cwt_frame_table.attach (cwt_mode_combobox, 1, 3, 0, 1);
  cwt_mode_combobox.signal_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  cwt_frame_table.attach (cwt_freq_res_label, 0, 1, 1, 2, Gtk::SHRINK);
  cwt_frame_table.attach (cwt_freq_res_scale, 1, 2, 1, 2);
  cwt_frame_table.attach (cwt_freq_res_value_label, 2, 3, 1, 2, Gtk::SHRINK);
  cwt_freq_res_label.set_text ("Frequency Resolution");
  cwt_freq_res_scale.set_draw_value (false);
  cwt_freq_res_scale.signal_value_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  cwt_frame_table.attach (cwt_time_res_label, 0, 1, 2, 3, Gtk::SHRINK);
  cwt_frame_table.attach (cwt_time_res_scale, 1, 2, 2, 3);
  cwt_frame_table.attach (cwt_time_res_value_label, 2, 3, 2, 3, Gtk::SHRINK);
  cwt_time_res_label.set_text ("Time Resolution");
  cwt_time_res_scale.set_draw_value (false);
  cwt_time_res_scale.signal_value_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  table.attach (progress_bar, 0, 3, 3, 4);

  frame_size_scale.set_value (0);
  frame_overlap_scale.set_value (0);
  cwt_freq_res_scale.set_value (25);
  cwt_time_res_scale.set_value (0);
  add (table);

  show_all_children();
}

AnalysisParams
FFTParamWindow::get_analysis_params()
{
  AnalysisParams params;

  if (transform_combobox.get_active_text() == TEXT_FFT)
    params.transform_type = SM_TRANSFORM_FFT;
  else if (transform_combobox.get_active_text() == TEXT_CWT)
    params.transform_type = SM_TRANSFORM_CWT;
  else if (transform_combobox.get_active_text() == TEXT_LPC)
    params.transform_type = SM_TRANSFORM_LPC;
  else
    assert (false);

  if (cwt_mode_combobox.get_active_text() == TEXT_VTIME)
    params.cwt_mode = SM_CWT_MODE_VTIME;
  else if (cwt_mode_combobox.get_active_text() == TEXT_CTIME)
    params.cwt_mode = SM_CWT_MODE_CTIME;
  else
    assert (false);

  params.frame_size_ms = get_frame_size();
  params.frame_step_ms = get_frame_step();
  params.cwt_freq_resolution = cwt_freq_res_scale.get_value();
  params.cwt_time_resolution = get_cwt_time_resolution();

  return params;
}
#endif

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

#if 0
void
FFTParamWindow::set_progress (double progress)
{
  progress_bar.set_fraction (progress);
}
#endif
