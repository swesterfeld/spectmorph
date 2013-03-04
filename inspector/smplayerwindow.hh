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

#ifndef SPECTMORPH_PLAYER_WINDOW_HH
#define SPECTMORPH_PLAYER_WINDOW_HH

#include <jack/jack.h>

#include <QWidget>
#include <QSlider>
#include <QLabel>

#include "smsimplejackplayer.hh"

namespace SpectMorph {

class Navigator;
class PlayerWindow : public QWidget
{
  Q_OBJECT

  Navigator          *navigator;
  QSlider            *volume_slider;
  QLabel             *volume_label;

  SimpleJackPlayer    jack_player;

public:
  PlayerWindow (Navigator *navigator);

public slots:
  void on_play_clicked();
  void on_stop_clicked();
  void on_volume_changed (int new_volume);
};

}

#endif

