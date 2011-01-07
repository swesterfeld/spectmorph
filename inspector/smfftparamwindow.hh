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


#ifndef SPECTMORPH_FFTPARAMWINDOW_HH
#define SPECTMORPH_FFTPARAMWINDOW_HH

#include <gtkmm.h>

#include "smspectrumview.hh"
#include "smtimefreqview.hh"
#include "smzoomcontroller.hh"

namespace SpectMorph {

class FFTParamWindow : public Gtk::Window
{
  Gtk::Table          table;

  Gtk::ComboBoxText   transform_combobox;
  Gtk::Label          transform_label;

  Gtk::Frame          fft_frame;
  Gtk::Table          fft_frame_table;

  Gtk::Frame          cwt_frame;
  Gtk::Table          cwt_frame_table;

  Gtk::HScale         frame_size_scale;
  Gtk::Label          frame_size_label;
  Gtk::Label          frame_size_value_label;

  Gtk::HScale         frame_overlap_scale;
  Gtk::Label          frame_overlap_label;
  Gtk::Label          frame_overlap_value_label;

  Gtk::HScale         cwt_freq_res_scale;
  Gtk::Label          cwt_freq_res_label;
  Gtk::Label          cwt_freq_res_value_label;

  Gtk::HScale         cwt_time_res_scale;
  Gtk::Label          cwt_time_res_label;
  Gtk::Label          cwt_time_res_value_label;

  Gtk::ProgressBar    progress_bar;

  void on_value_changed();

  double get_frame_overlap();
  double get_frame_size();
  double get_frame_step();

  double get_cwt_time_resolution();

public:
  FFTParamWindow();

  void   set_progress (double progress);
  AnalysisParams get_analysis_params();

  sigc::signal<void> signal_params_changed;
};

}

#endif
