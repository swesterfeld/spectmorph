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

#include "smsamplewindow.hh"
#include "smnavigator.hh"
#include "smmemout.hh"
#include "smmmapin.hh"
#include "smlivedecoder.hh"

#include <iostream>
#include <jack/jack.h>

using namespace SpectMorph;

using std::vector;
using Birnet::AutoLocker;

class Source : public LiveDecoderSource
{
  Audio *my_audio;
public:
  Source (Audio *my_audio);

  void retrigger (int, float, int, float);
  Audio* audio();
  AudioBlock* audio_block (size_t index);
};

Source::Source (Audio *audio) :
  my_audio (audio)
{
}

void
Source::retrigger (int, float, int, float)
{
}

Audio *
Source::audio()
{
  return my_audio;
}

AudioBlock *
Source::audio_block (size_t index)
{
  if (my_audio && index < my_audio->contents.size())
    return &my_audio->contents[index];
  else
    return NULL;
}

int
jack_process (jack_nframes_t nframes, void *arg)
{
  PlayerWindow *instance = reinterpret_cast<PlayerWindow *> (arg);
  return instance->process (nframes);
}

int
PlayerWindow::process (jack_nframes_t nframes)
{
  AutoLocker lock (decoder_mutex);

  float *audio_out = (jack_default_audio_sample_t *) jack_port_get_buffer (audio_out_port, nframes);
  if (decoder)
    {
      decoder->process (nframes, 0, 0, audio_out);
      for (size_t i = 0; i < nframes; i++)
        audio_out[i] *= decoder_volume;
    }
  else
    {
      for (size_t i = 0; i < nframes; i++)
        {
          audio_out[i] = 0;
        }
    }
  return 0;
}

PlayerWindow::PlayerWindow (Navigator *navigator) :
  navigator (navigator),
  decoder (NULL),
  decoder_audio (NULL),
  decoder_source (NULL),
  decoder_volume (0), // will be initialized by scale widget
  volume_scale (-96, 24, 0.01)
{
  set_border_width (10);
  set_default_size (300, 100);
  set_title ("Player");

  play_button.set_label ("Play");
  play_button.signal_clicked().connect (sigc::mem_fun (*this, &PlayerWindow::on_play_clicked));

  stop_button.set_label ("Stop");
  stop_button.signal_clicked().connect (sigc::mem_fun (*this, &PlayerWindow::on_stop_clicked));

  volume_scale.signal_value_changed().connect (sigc::mem_fun (*this, &PlayerWindow::on_volume_changed));

  button_hbox.pack_start (play_button);
  button_hbox.pack_start (stop_button);
  button_hbox.set_spacing (10);

  volume_label.set_label ("Volume");
  volume_scale.set_value (-20);
  volume_scale.set_draw_value (false);

  volume_hbox.pack_start (volume_label, Gtk::PACK_SHRINK);
  volume_hbox.pack_start (volume_scale);
  volume_hbox.pack_start (volume_value_label, Gtk::PACK_SHRINK);

  vbox.add (button_hbox);
  vbox.add (volume_hbox);

  add (vbox);

  show_all_children();

  jack_client = jack_client_open ("sminspector", JackNullOption, NULL);

  jack_set_process_callback (jack_client, jack_process, this);

  audio_out_port = jack_port_register (jack_client, "audio_out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
  if (jack_activate (jack_client))
    {
      fprintf (stderr, "cannot activate client");
      exit (1);
    }
  jack_mix_freq = jack_get_sample_rate (jack_client);
}

PlayerWindow::~PlayerWindow()
{
  jack_deactivate (jack_client);

  // delete old decoder objects (if any)
  update_decoder (NULL, NULL, NULL);
}

void
PlayerWindow::on_play_clicked()
{
  LiveDecoder       *new_decoder        = NULL;
  Audio             *new_decoder_audio  = NULL;
  LiveDecoderSource *new_decoder_source = NULL;

  Audio *audio = navigator->get_audio();
  if (audio)
    {
      // create a deep copy (by saving/loading), so that JACK thread can access data in JACK thread

      vector<unsigned char> audio_data;
      MemOut                audio_mo (&audio_data);
      audio->save (&audio_mo);

      new_decoder_audio = new Audio();
      GenericIn *in = MMapIn::open_mem (&audio_data[0], &audio_data[audio_data.size()]);
      new_decoder_audio->load (in);
      delete in;

      new_decoder_source = new Source (new_decoder_audio);

      new_decoder = new LiveDecoder (new_decoder_source);
      new_decoder->retrigger (/* channel */ 0, audio->fundamental_freq, 127, jack_mix_freq);

      // touch decoder in non-RT-thread to precompute tables & co
      vector<float> samples (10000);
      new_decoder->process (samples.size(), 0, 0, &samples[0]);

      // finally setup decoder for JACK thread
      new_decoder->retrigger (/* channel */ 0, audio->fundamental_freq, 127, jack_mix_freq);
    }
  update_decoder (new_decoder, new_decoder_audio, new_decoder_source);
}

void
PlayerWindow::update_decoder (LiveDecoder *new_decoder, Audio *new_decoder_audio, LiveDecoderSource *new_decoder_source)
{
  LiveDecoder       *old_decoder;
  Audio             *old_decoder_audio;
  LiveDecoderSource *old_decoder_source;

  /* setup new player objects for JACK thread */
  decoder_mutex.lock();

  old_decoder = decoder;
  old_decoder_source = decoder_source;
  old_decoder_audio = decoder_audio;

  decoder = new_decoder;
  decoder_source = new_decoder_source;
  decoder_audio = new_decoder_audio;

  decoder_mutex.unlock();

  /* delete old (no longer needed) player objects */
  if (old_decoder)
    delete old_decoder;
  if (old_decoder_audio)
    delete old_decoder_audio;
  if (old_decoder_source)
    delete old_decoder_source;
}

void
PlayerWindow::on_stop_clicked()
{
  update_decoder (NULL, NULL, NULL);
}

void
PlayerWindow::on_volume_changed()
{
  double new_decoder_volume = bse_db_to_factor (volume_scale.get_value());
  volume_value_label.set_text (Birnet::string_printf ("%.1f dB", volume_scale.get_value()));

  AutoLocker lock (decoder_mutex);
  decoder_volume = new_decoder_volume;
}
