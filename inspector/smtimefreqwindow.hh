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

#ifndef SPECTMORPH_TIME_FREQ_WINDOW_HH
#define SPECTMORPH_TIME_FREQ_WINDOW_HH

#include "smtimefreqview.hh"
#include "smzoomcontroller.hh"
#include "smspectrumwindow.hh"
#include "smfftparamwindow.hh"
#include "smsamplewindow.hh"

#include <QScrollArea>

namespace SpectMorph {

class Navigator;
class TimeFreqWindow : public QWidget
{
  Q_OBJECT

  QLabel             *position_label;
  QSlider            *position_slider;
  QScrollArea        *scroll_area;
  TimeFreqView       *m_time_freq_view;
  ZoomController     *zoom_controller;

  Navigator          *navigator;

public:
  TimeFreqWindow (Navigator *navigator);

public slots:
  void on_dhandle_changed();
  void on_zoom_changed();
  void on_position_changed();

#if 0
  Gtk::ScrolledWindow scrolled_win;
  TimeFreqView        m_time_freq_view;
  ZoomController      zoom_controller;

  Gtk::Adjustment     position_adjustment;
  Gtk::HScale         position_scale;
  Gtk::Label          position_label;
  Gtk::HBox           position_hbox;

  Gtk::HScale         min_db_scale;
  Gtk::Label          min_db_label;
  Gtk::HBox           min_db_hbox;

  Gtk::HScale         boost_scale;
  Gtk::Label          boost_label;
  Gtk::HBox           boost_hbox;

  Gtk::VBox           vbox;

public:
  TimeFreqWindow (Navigator *navigator);

  void on_analysis_changed();
  void on_frequency_grid_changed();
  void on_progress_changed();
  void on_display_params_changed();
  void on_resized (int old_width, int old_height, int new_width, int new_height);

  TimeFreqView *time_freq_view();
#endif
};

}

#endif
