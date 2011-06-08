/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
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

#include "smlpcwindow.hh"

using namespace SpectMorph;

LPCWindow::LPCWindow()
{
  set_border_width (10);
  set_default_size (600, 600);
  set_title ("LPC View");

  vbox.pack_start (scrolled_win);
  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);

  scrolled_win.add (lpc_view);
  add (vbox);

  show_all_children();

  //zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &SpectrumWindow::on_zoom_changed));
}

void
LPCWindow::set_lpc_model (TimeFreqView& time_freq_view)
{
  lpc_view.set_lpc_model (time_freq_view);
}
