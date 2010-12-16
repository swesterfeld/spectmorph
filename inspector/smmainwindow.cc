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

#include "smmainwindow.hh"
#include "smmath.hh"

using std::vector;
using std::string;

using namespace SpectMorph;

MainWindow::MainWindow (const string& filename) :
  //time_freq_view (filename),
  position_adjustment (0.0, 0.0, 1.0, 0.01, 1.0, 0.0),
  position_scale (position_adjustment),
  navigator (filename)
{
  set_border_width (10);
  set_default_size (800, 600);
  vbox.pack_start (scrolled_win);

  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);
  zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &MainWindow::on_zoom_changed));

  vbox.pack_start (position_hbox, Gtk::PACK_SHRINK);
  position_hbox.pack_start (position_scale);
  position_hbox.pack_start (position_label, Gtk::PACK_SHRINK);
  position_scale.set_draw_value (false);
  position_label.set_text ("frame 0");
  position_hbox.set_border_width (10);
  position_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MainWindow::on_position_changed));

  add (vbox);
  scrolled_win.add (time_freq_view);
  show_all_children();

  navigator.signal_dhandle_changed.connect (sigc::mem_fun (*this, &MainWindow::on_dhandle_changed));
  navigator.signal_show_position_changed.connect (sigc::mem_fun (*this, &MainWindow::on_position_changed));

  spectrum_window.set_spectrum_model (time_freq_view);
}

void
MainWindow::on_zoom_changed()
{
  time_freq_view.set_zoom (zoom_controller.get_hzoom(), zoom_controller.get_vzoom());
}

void
MainWindow::on_position_changed()
{
  int frames = time_freq_view.get_frames();
  int position = CLAMP (sm_round_positive (position_adjustment.get_value() * frames), 0, frames - 1);
  char buffer[1024];
  sprintf (buffer, "frame %d", position);
  position_label.set_text (buffer);
  if (navigator.get_show_position())
    time_freq_view.set_position (position);
  else
    time_freq_view.set_position (-1);
}

void
MainWindow::on_dhandle_changed()
{
  time_freq_view.load (navigator.get_dhandle(), "fn");
}
