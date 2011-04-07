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

#include <gtkmm.h>
#include <jack/jack.h>

#include "smlivedecoder.hh"

namespace SpectMorph {

class Navigator;
class PlayerWindow : public Gtk::Window
{
  Navigator          *navigator;
  Gtk::Button         play_button;
  jack_port_t        *audio_out_port;
  jack_client_t      *jack_client;
  LiveDecoder        *decoder;
  double              jack_mix_freq;
  Birnet::Mutex       decoder_mutex;
public:
  PlayerWindow (Navigator *navigator);
  ~PlayerWindow();

  int  process (jack_nframes_t nframes);
  void on_play_clicked();
};

}

#endif

