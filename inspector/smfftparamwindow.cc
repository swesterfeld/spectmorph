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
#include <birnet/birnet.hh>

using namespace SpectMorph;

FFTParamWindow::FFTParamWindow() :
  table (2, 3),
  frame_size_scale (-1, 1, 0.01),
  frame_step_scale (-1, 1, 0.01)
{
  set_border_width (10);
  set_default_size (500, 200);
  set_title ("FFT Parameters");

  table.attach (frame_size_label, 0, 1, 0, 1, Gtk::SHRINK);
  table.attach (frame_size_scale, 1, 2, 0, 1);
  table.attach (frame_size_value_label, 2, 3, 0, 1, Gtk::SHRINK);
  frame_size_label.set_text ("Frame Size");
  frame_size_scale.set_draw_value (false);
  frame_size_scale.signal_value_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  table.attach (frame_step_label, 0, 1, 1, 2, Gtk::SHRINK);
  table.attach (frame_step_scale, 1, 2, 1, 2);
  table.attach (frame_step_value_label, 2, 3, 1, 2, Gtk::SHRINK);
  frame_step_label.set_text ("Frame Step");
  frame_step_scale.set_draw_value (false);
  frame_step_scale.signal_value_changed().connect (sigc::mem_fun (*this, &FFTParamWindow::on_value_changed));

  frame_size_scale.set_value (0);
  frame_step_scale.set_value (0);
  add (table);

  show();
  show_all_children();
}

double
FFTParamWindow::get_frame_size()
{
  return 40 * pow (10, frame_size_scale.get_value());
}

double
FFTParamWindow::get_frame_step()
{
  return 10 * pow (10, frame_step_scale.get_value());
}

void
FFTParamWindow::on_value_changed()
{
  frame_size_value_label.set_text (Birnet::string_printf ("%.1f ms", get_frame_size()));
  frame_step_value_label.set_text (Birnet::string_printf ("%.1f ms", get_frame_step()));

  signal_params_changed();
}
