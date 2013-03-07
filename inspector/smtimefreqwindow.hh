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
  QLabel             *min_db_label;
  QSlider            *min_db_slider;
  QLabel             *boost_label;
  QSlider            *boost_slider;
  QScrollArea        *scroll_area;
  TimeFreqView       *m_time_freq_view;
  ZoomController     *zoom_controller;

  Navigator          *navigator;

public:
  TimeFreqWindow (Navigator *navigator);

  TimeFreqView *time_freq_view();

public slots:
  void on_dhandle_changed();
  void on_zoom_changed();
  void on_position_changed();
  void on_display_params_changed();
  void on_progress_changed();
  void on_analysis_changed();
  void on_frequency_grid_changed();
};

}

#endif
