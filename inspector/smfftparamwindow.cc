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
  table (2, 3),
  frame_size_scale (-1, 1, 0.01),
  frame_overlap_scale (-1, 2, 0.01)
{
  set_border_width (10);
  set_default_size (500, 200);
  set_title ("FFT Parameters");

  table.attach (transform_label, 0, 1, 0, 1, Gtk::SHRINK);
  table.attach (transform_combobox, 1, 3, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND);
  transform_label.set_text ("Transform Type");
  transform_combobox.append_text ("Fourier Transform");
  transform_combobox.append_text ("Wavelet Transform");
  transform_combobox.set_active_text ("Fourier Transform");
  transform_combobox.signal_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  table.attach (frame_size_label, 0, 1, 1, 2, Gtk::SHRINK);
  table.attach (frame_size_scale, 1, 2, 1, 2);
  table.attach (frame_size_value_label, 2, 3, 1, 2, Gtk::SHRINK);
  frame_size_label.set_text ("FFT Frame Size");
  frame_size_scale.set_draw_value (false);
  frame_size_scale.signal_value_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  table.attach (frame_overlap_label, 0, 1, 2, 3, Gtk::SHRINK);
  table.attach (frame_overlap_scale, 1, 2, 2, 3);
  table.attach (frame_overlap_value_label, 2, 3, 2, 3, Gtk::SHRINK);
  frame_overlap_label.set_text ("FFT Frame Overlap");
  frame_overlap_scale.set_draw_value (false);
  frame_overlap_scale.signal_value_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  frame_size_scale.set_value (0);
  frame_overlap_scale.set_value (0);
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

  return params;
}

double
FFTParamWindow::get_frame_size()
{
  return 40 * pow (10, frame_size_scale.get_value());
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

  signal_params_changed();
}
