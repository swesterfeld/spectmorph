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
  set_title ("Sample View");

  vbox.pack_start (scrolled_win);
  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);

  add (vbox);
  scrolled_win.add (sample_view);

  zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &SampleWindow::on_zoom_changed));
  sample_view.signal_resized.connect (sigc::mem_fun (*this, &SampleWindow::on_resized));

  show_all_children();
  show();
}

void
SampleWindow::load (GslDataHandle *dhandle, Audio *audio)
{
  sample_view.load (dhandle, audio);
}

void
SampleWindow::on_zoom_changed()
{
  sample_view.set_zoom (zoom_controller.get_hzoom(), zoom_controller.get_vzoom());
}

void
SampleWindow::on_resized (int old_width, int old_height, int new_width, int new_height)
{
  if (old_width > 0 && old_height > 0 && new_width > 0 && new_height > 0)
    {
      Gtk::Viewport *view_port = dynamic_cast<Gtk::Viewport*> (scrolled_win.get_child());
      const int h_2 = view_port->get_height() / 2;
      const int w_2 = view_port->get_width() / 2;
      Gtk::Adjustment *vadj = scrolled_win.get_vadjustment();
      Gtk::Adjustment *hadj = scrolled_win.get_hadjustment();

      vadj->set_value ((vadj->get_value() + h_2) / old_height * new_height - h_2);
      hadj->set_value ((hadj->get_value() + w_2) / old_width * new_width - w_2);
    }
}
