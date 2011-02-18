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

using namespace SpectMorph;

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
  transform_combobox.append_text ("Fourier Transform");
  transform_combobox.append_text ("Wavelet Transform");
  transform_combobox.set_active_text ("Fourier Transform");
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

  cwt_frame_table.attach (cwt_freq_res_label, 0, 1, 0, 1, Gtk::SHRINK);
  cwt_frame_table.attach (cwt_freq_res_scale, 1, 2, 0, 1);
  cwt_frame_table.attach (cwt_freq_res_value_label, 2, 3, 0, 1, Gtk::SHRINK);
  cwt_freq_res_label.set_text ("Frequency Resolution");
  cwt_freq_res_scale.set_draw_value (false);
  cwt_freq_res_scale.signal_value_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  cwt_frame_table.attach (cwt_time_res_label, 0, 1, 1, 2, Gtk::SHRINK);
  cwt_frame_table.attach (cwt_time_res_scale, 1, 2, 1, 2);
  cwt_frame_table.attach (cwt_time_res_value_label, 2, 3, 1, 2, Gtk::SHRINK);
  cwt_time_res_label.set_text ("Time Resolution");
  cwt_time_res_scale.set_draw_value (false);
  cwt_time_res_scale.signal_value_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  table.attach (progress_bar, 0, 3, 3, 4);

  frame_size_scale.set_value (0);
  frame_overlap_scale.set_value (0);
  cwt_freq_res_scale.set_value (25);
  cwt_time_res_scale.set_value (0);
  add (table);

  show();
  show_all_children();
}

AnalysisParams
FFTParamWindow::get_analysis_params()
{
  AnalysisParams params;

  if (transform_combobox.get_active_text() == "Fourier Transform")
    params.transform_type = SM_TRANSFORM_FFT;
  else if (transform_combobox.get_active_text() == "Wavelet Transform")
    params.transform_type = SM_TRANSFORM_CWT;
  else
    assert (false);

  params.frame_size_ms = get_frame_size();
  params.frame_step_ms = get_frame_step();
  params.cwt_freq_resolution = cwt_freq_res_scale.get_value();
  params.cwt_time_resolution = get_cwt_time_resolution();

  return params;
}

double
FFTParamWindow::get_frame_size()
{
  return 40 * pow (10, frame_size_scale.get_value());
}

double
FFTParamWindow::get_cwt_time_resolution()
{
  return 40 * pow (10, cwt_time_res_scale.get_value());
}

double
FFTParamWindow::get_frame_step()
{
  return get_frame_size() / get_frame_overlap();
}

double
FFTParamWindow::get_frame_overlap()
{
  return pow (4, frame_overlap_scale.get_value() + 1);
}

void
FFTParamWindow::on_value_changed()
{
  frame_size_value_label.set_text (Birnet::string_printf ("%.1f ms", get_frame_size()));
  frame_overlap_value_label.set_text (Birnet::string_printf ("%.1f", get_frame_overlap()));
  cwt_freq_res_value_label.set_text (Birnet::string_printf ("%.1f Hz", cwt_freq_res_scale.get_value()));
  cwt_time_res_value_label.set_text (Birnet::string_printf ("%.1f ms", get_cwt_time_resolution()));

  signal_params_changed();
}

void
FFTParamWindow::set_progress (double progress)
{
  progress_bar.set_fraction (progress);
}
