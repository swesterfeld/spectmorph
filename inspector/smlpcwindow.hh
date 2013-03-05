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


#ifndef SPECTMORPH_LPC_WINDOW_HH
#define SPECTMORPH_LPC_WINDOW_HH

#include "smlpcview.hh"
#include "smtimefreqview.hh"
#include "smzoomcontroller.hh"

#include <QScrollArea>

namespace SpectMorph {

class LPCWindow : public QWidget
{
  Q_OBJECT

  QScrollArea      *scroll_area;
  LPCView          *lpc_view;
  ZoomController   *zoom_controller;

public:
  LPCWindow();

  void set_lpc_model (TimeFreqView *time_freq_view);

public slots:
  void on_zoom_changed();
};

}

#endif
