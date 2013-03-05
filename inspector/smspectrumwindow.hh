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


#ifndef SPECTMORPH_SPECTRUMWINDOW_HH
#define SPECTMORPH_SPECTRUMWINDOW_HH

#include "smspectrumview.hh"
#include "smtimefreqview.hh"
#include "smzoomcontroller.hh"

#include <QScrollArea>

namespace SpectMorph {

class Navigator;
class SpectrumWindow : public QWidget
{
  Q_OBJECT

  QScrollArea        *scroll_area;
  ZoomController     *zoom_controller;
  SpectrumView       *spectrum_view;
public:
  SpectrumWindow (Navigator *navigator);

  void set_spectrum_model (TimeFreqView *time_freq_view);
public slots:
  void on_zoom_changed();
};

}

#endif
