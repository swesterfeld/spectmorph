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

#include "smsamplewindow.hh"

using namespace SpectMorph;

SampleWindow::SampleWindow()
{
  set_border_width (10);
  set_default_size (800, 600);

  vbox.pack_start (scrolled_win);
  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);

  add (vbox);
  scrolled_win.add (sample_view);

  show_all_children();
  show();
}

void
SampleWindow::load (GslDataHandle *dhandle, Audio *audio)
{
  sample_view.load (dhandle, audio);
}
