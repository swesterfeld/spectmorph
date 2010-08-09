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

#include "smwavset.hh"

#include <jack/jack.h>
#include <jack/midiport.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

using SpectMorph::WavSet;
using SpectMorph::WavSetWave;
using SpectMorph::Audio;

using std::vector;
using std::string;

class JackSynth
{
protected:
  double        jack_mix_freq;
  jack_port_t  *input_port;
  jack_port_t  *output_port;
  WavSet       *smset;

public:
  void init (jack_client_t *client, WavSet *wset);
  int  process (jack_nframes_t nframes);
};

static bool
is_note_on (const jack_midi_event_t& event)
{
  if ((event.buffer[0] & 0xf0) == 0x90)
    {
      if (event.buffer[2] != 0) /* note on with velocity 0 => note off */
        return true;
    }
  return false;
}

static bool
is_note_off (const jack_midi_event_t& event)
{
  if ((event.buffer[0] & 0xf0) == 0x90)
    {
      if (event.buffer[2] == 0) /* note on with velocity 0 => note off */
        return true;
    }
  else if ((event.buffer[0] & 0xf0) == 0x80)
    {
      return true;
    }
  return false;
}

int
JackSynth::process (jack_nframes_t nframes)
{
  void* port_buf = jack_port_get_buffer (input_port, nframes);
  jack_nframes_t event_count = jack_midi_get_event_count (port_buf);
  jack_midi_event_t in_event;
  jack_nframes_t event_index = 0;

  jack_midi_event_get (&in_event, port_buf, 0);
  for (jack_nframes_t i = 0; i < nframes; i++)
    {
      while ((in_event.time == i) && (event_index < event_count))
        {
          if (is_note_on (in_event))
            {
              int    midi_note = in_event.buffer[1];
              double velocity = in_event.buffer[2] / 127.0;

              printf ("note on: %d %f\n", midi_note, velocity);

              Audio *best_audio = NULL;
              int    best_diff  = 200;
              string best_path;

              for (vector<WavSetWave>::iterator wi = smset->waves.begin(); wi != smset->waves.end(); wi++)
                {
                  int diff = abs (wi->midi_note - midi_note);
                  if (diff < best_diff)
                    {
                      best_audio = wi->audio;
                      best_path  = wi->path;
                      best_diff  = diff;
                    }
                }
              printf (" => %s\n", best_path.c_str());
            }
          else if (is_note_off (in_event))
            {
              printf ("note off: %d\n", in_event.buffer[1]);
            }

          // get next event
          event_index++;
          if (event_index < event_count)
            jack_midi_event_get (&in_event, port_buf, event_index);
        }
    }
  return 0;
}

int
jack_process (jack_nframes_t nframes, void *arg)
{
  JackSynth *instance = reinterpret_cast<JackSynth *> (arg);
  return instance->process (nframes);
}

void
JackSynth::init (jack_client_t *client, WavSet *smset)
{
  this->smset = smset;

  jack_set_process_callback (client, jack_process, this);

  jack_mix_freq = jack_get_sample_rate (client);

  input_port = jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
  output_port = jack_port_register (client, "audio_out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

  if (jack_activate (client))
    {
      fprintf (stderr, "cannot activate client");
      exit (1);
    }
}

int
main(int argc, char **argv)
{
  if (argc != 2)
    {
      printf ("usage: smjack <smset_filename>\n");
      exit (1);
    }

  printf ("loading %s ...", argv[1]);
  fflush (stdout);

  WavSet wset;
  BseErrorType error = wset.load (argv[1]);
  if (error)
    {
      printf ("\n");
      fprintf (stderr, "%s: can't open input file: %s: %s\n", argv[0], argv[1], bse_error_blurb (error));
      exit (1);
    }
  printf ("%zd audio entries found.\n", wset.waves.size());

  jack_client_t *client;
  client = jack_client_open ("smjack", JackNullOption, NULL);

  if (!client)
    {
      fprintf (stderr, "unable to connect to jack server\n");
      exit (1);
    }

  JackSynth synth;
  synth.init (client, &wset);

  while (1)
    {
      sleep (1);
    }
}
