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

#ifndef SPECTMORPH_MAIN_WINDOW_HH
#define SPECTMORPH_MAIN_WINDOW_HH

#include <gtkmm.h>

#include "smtimefreqview.hh"
#include "smzoomcontroller.hh"
#include "smnavigator.hh"
#include "smspectrumwindow.hh"

namespace SpectMorph {

class MainWindow : public Gtk::Window
{
  Gtk::ScrolledWindow scrolled_win;
  TimeFreqView        time_freq_view;
  ZoomController      zoom_controller;
  Gtk::Adjustment     position_adjustment;
  Gtk::HScale         position_scale;
  Gtk::Label          position_label;
  Gtk::HBox           position_hbox;
  Gtk::VBox           vbox;
  Navigator           navigator;
  SpectrumWindow      spectrum_window;

public:
  MainWindow (const std::string& filename);

  void on_zoom_changed();
  void on_dhandle_changed();
  void on_position_changed();
  void on_analysis_changed();
};

}

#endif
