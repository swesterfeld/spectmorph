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

#include "smspectrumwindow.hh"
#include "smnavigator.hh"

using namespace SpectMorph;

#if 0
SpectrumWindow::SpectrumWindow (Navigator *navigator) :
  spectrum_view (navigator),
  zoom_controller (5000, 1000)
{
  set_border_width (10);
  set_default_size (800, 600);
  set_title ("Spectrum View");

  vbox.pack_start (scrolled_win);
  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);

  scrolled_win.add (spectrum_view);
  add (vbox);

  show_all_children();

  zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &SpectrumWindow::on_zoom_changed));
  navigator->display_param_window()->signal_params_changed.connect (sigc::mem_fun (spectrum_view, &SpectrumView::on_display_params_changed));
}

void
SpectrumWindow::set_spectrum_model (TimeFreqView& time_freq_view)
{
  spectrum_view.set_spectrum_model (time_freq_view);
}

void
SpectrumWindow::on_zoom_changed()
{
  spectrum_view.set_zoom (zoom_controller.get_hzoom(), zoom_controller.get_vzoom());
}
#endif
