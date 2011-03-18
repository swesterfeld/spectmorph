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

#include "smzoomcontroller.hh"

using namespace SpectMorph;

ZoomController::ZoomController (double hzoom_max, double vzoom_max) :
  hzoom_adjustment (0.0, -1.0, log10 (hzoom_max) - 2, 0.01, 1.0, 0.0),
  hzoom_scale (hzoom_adjustment),
  vzoom_adjustment (0.0, -1.0, log10 (vzoom_max) - 2, 0.01, 1.0, 0.0),
  vzoom_scale (vzoom_adjustment)
{
  pack_start (hzoom_hbox, Gtk::PACK_SHRINK);
  hzoom_hbox.pack_start (hzoom_scale);
  hzoom_hbox.pack_start (hzoom_label, Gtk::PACK_SHRINK);
  hzoom_scale.set_draw_value (false);
  hzoom_label.set_text ("100.00%");
  hzoom_hbox.set_border_width (10);
  hzoom_scale.signal_value_changed().connect (sigc::mem_fun (*this, &ZoomController::on_hzoom_changed));

  pack_start (vzoom_hbox, Gtk::PACK_SHRINK);
  vzoom_hbox.pack_start (vzoom_scale);
  vzoom_hbox.pack_start (vzoom_label, Gtk::PACK_SHRINK);
  vzoom_scale.set_draw_value (false);
  vzoom_label.set_text ("100.00%");
  vzoom_hbox.set_border_width (10);
  vzoom_scale.signal_value_changed().connect (sigc::mem_fun (*this, &ZoomController::on_vzoom_changed));
}

double
ZoomController::get_hzoom()
{
  return pow (10, hzoom_adjustment.get_value());
}

double
ZoomController::get_vzoom()
{
  return pow (10, vzoom_adjustment.get_value());
}

void
ZoomController::on_hzoom_changed()
{
  double hzoom = get_hzoom();
  char buffer[1024];
  sprintf (buffer, "%3.2f%%", 100.0 * hzoom);
  hzoom_label.set_text (buffer);

  signal_zoom_changed();
}

void
ZoomController::on_vzoom_changed()
{
  double vzoom = get_vzoom();
  char buffer[1024];
  sprintf (buffer, "%3.2f%%", 100.0 * vzoom);
  vzoom_label.set_text (buffer);

  signal_zoom_changed();
}


