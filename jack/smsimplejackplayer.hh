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

#ifndef SPECTMORPH_SIMPLE_JACK_PLAYER_HH
#define SPECTMORPH_SIMPLE_JACK_PLAYER_HH

#include <jack/jack.h>

#include "smlivedecoder.hh"

namespace SpectMorph
{

class SimpleJackPlayer
{
  jack_port_t        *audio_out_port;
  jack_client_t      *jack_client;

  Birnet::Mutex       decoder_mutex;
  LiveDecoder        *decoder;            // decoder_mutex!
  Audio              *decoder_audio;      // decoder_mutex!
  LiveDecoderSource  *decoder_source;     // decoder_mutex!
  double              decoder_volume;     // decoder_mutex!

  double              jack_mix_freq;

  void update_decoder (LiveDecoder *new_decoder, Audio *new_decoder_audio, LiveDecoderSource *new_decoder_source);
public:
  SimpleJackPlayer (const std::string& client_name);
  ~SimpleJackPlayer();

  void play (Audio *audio, bool use_samples);
  int  process (jack_nframes_t nframes);
  void set_volume (double new_volume);
};

}

#endif
