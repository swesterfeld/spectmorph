/*
 * Copyright (C) 2011 Stefan Westerfeld
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

#ifndef SPECTMORPH_SAMPLE_WINDOW_HH
#define SPECTMORPH_SAMPLE_WINDOW_HH

#include "smsampleview.hh"
#include "smzoomcontroller.hh"
#include "smwavset.hh"

#include <QMainWindow>

namespace SpectMorph {

class Navigator;
class SampleWinView;
class SampleWindow : public QMainWindow
{
  Q_OBJECT

private:
  Navigator          *navigator;
  SampleWinView      *sample_win_view;

public:
  SampleWindow (Navigator *navigator);

  void        load (GslDataHandle *dhandle, Audio *audio);
  SampleView *sample_view();

public slots:
  void on_dhandle_changed();
  void on_next_sample();

signals:
  void next_sample();
};

}

#endif
