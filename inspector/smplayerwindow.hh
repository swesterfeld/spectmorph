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
  Gtk::Button         stop_button;
  Gtk::HBox           button_hbox;
  Gtk::HBox           volume_hbox;
  Gtk::VBox           vbox;

  jack_port_t        *audio_out_port;
  jack_client_t      *jack_client;
  LiveDecoder        *decoder;            // decoder_mutex!
  Audio              *decoder_audio;      // decoder_mutex!
  LiveDecoderSource  *decoder_source;     // decoder_mutex!
  double              decoder_volume;     // decoder_mutex!
  double              jack_mix_freq;
  Birnet::Mutex       decoder_mutex;

  Gtk::HScale         volume_scale;
  Gtk::Label          volume_label;
  Gtk::Label          volume_value_label;

  void update_decoder (LiveDecoder *new_decoder, Audio *new_decoder_audio, LiveDecoderSource *new_decoder_source);
public:
  PlayerWindow (Navigator *navigator);
  ~PlayerWindow();

  int  process (jack_nframes_t nframes);
  void on_play_clicked();
  void on_stop_clicked();
  void on_volume_changed();
};

}

#endif

