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

#ifndef SPECTMORPH_ZOOMCONTROLLER_HH
#define SPECTMORPH_ZOOMCONTROLLER_HH

namespace SpectMorph {

class ZoomController //: public Gtk::VBox
{
#if 0
  Gtk::Adjustment     hzoom_adjustment;
  Gtk::HScale         hzoom_scale;
  Gtk::Label          hzoom_label;
  Gtk::HBox           hzoom_hbox;
  Gtk::Adjustment     vzoom_adjustment;
  Gtk::HScale         vzoom_scale;
  Gtk::Label          vzoom_label;
  Gtk::HBox           vzoom_hbox;

  void init();
public:
  ZoomController (double hzoom_max = 1000.0, double vzoom_max = 1000.0);
  ZoomController (double hzoom_min, double hzoom_max, double vzoom_min, double vzoom_max);

  void on_hzoom_changed();
  void on_vzoom_changed();

  double get_hzoom();
  double get_vzoom();

  sigc::signal<void> signal_zoom_changed;
#endif
};

}

#endif
