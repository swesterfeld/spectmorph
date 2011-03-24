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
#include "smlivedecoder.hh"
#include "smmorphplan.hh"
#include "smmorphplanview.hh"
#include "smmorphplanwindow.hh"

#include <jack/jack.h>
#include <jack/midiport.h>
#include <gtkmm.h>

#include "smmain.hh"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::max;
using std::min;

class Voice
{
public:
  enum State {
    STATE_IDLE,
    STATE_ON,
    STATE_RELEASE
  };
  vector<LiveDecoder *> decoders;

  State        state;
  bool         pedal;
  int          midi_note;
  double       env;
  double       velocity;

  Voice() :
    state (STATE_IDLE),
    pedal (false)
  {
  }
  ~Voice()
  {
    for (vector<LiveDecoder *>::iterator di = decoders.begin(); di != decoders.end(); di++)
      delete *di;
    decoders.clear();
  }
};

class JackSynth
{
protected:
  double                jack_mix_freq;
  jack_port_t          *input_port;
  vector<jack_port_t *> output_ports;
  WavSet               *smset;
  int                   channels;
  bool                  need_reschedule;
  bool                  pedal_down;

  double                release_ms;
  vector<Voice>         voices;
  vector<Voice*>        active_voices;
  vector<Voice*>        release_voices;

public:
  JackSynth();
  void init (jack_client_t *client, WavSet *wset, int channels);
  int  process (jack_nframes_t nframes);
  void reschedule();
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

static float
freq_from_note (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

void
JackSynth::reschedule()
{
  active_voices.clear();
  release_voices.clear();

  for (vector<Voice>::iterator vi = voices.begin(); vi != voices.end(); vi++)
    {
      if (vi->state == Voice::STATE_ON)
        active_voices.push_back (&*vi);
      else if (vi->state == Voice::STATE_RELEASE)
        release_voices.push_back (&*vi);
    }
}

int
JackSynth::process (jack_nframes_t nframes)
{
  vector<jack_default_audio_sample_t *> outputs (channels);  /* FIXME: could be malloc-free */
  for (int c = 0; c < channels; c++)
    {
      outputs[c] = (jack_default_audio_sample_t *) jack_port_get_buffer (output_ports[c], nframes);

      // zero output buffer, so voices can be added
      zero_float_block (nframes, outputs[c]);
    }

  void* port_buf = jack_port_get_buffer (input_port, nframes);
  jack_nframes_t event_count = jack_midi_get_event_count (port_buf);
  jack_midi_event_t in_event;
  jack_nframes_t event_index = 0;

  jack_midi_event_get (&in_event, port_buf, 0);
  jack_nframes_t i = 0; 
  while (i < nframes)
    {
      while ((in_event.time == i) && (event_index < event_count))
        {
          if (is_note_on (in_event))
            {
              const double master_volume = 0.333;    /* empiric */

              const int midi_note = in_event.buffer[1];
              const int midi_velocity = in_event.buffer[2];

              double velocity = midi_velocity / 127.0 * master_volume;

              // find unused voice
              vector<Voice>::iterator vi = voices.begin();
              while (vi != voices.end() && vi->state != Voice::STATE_IDLE)
                vi++;
              if (vi != voices.end())
                {
                  for (int c = 0; c < channels; c++)
                    vi->decoders[c]->retrigger (c, freq_from_note (midi_note), midi_velocity, jack_mix_freq);
                  vi->state = Voice::STATE_ON;
                  vi->midi_note = midi_note;
                  vi->velocity = velocity;
                  need_reschedule = true;
                }
            }
          else if (is_note_off (in_event))
            {
              int    midi_note = in_event.buffer[1];

              for (vector<Voice>::iterator vi = voices.begin(); vi != voices.end(); vi++)
                {
                  if (vi->state == Voice::STATE_ON && vi->midi_note == midi_note)
                    {
                      if (pedal_down)
                        {
                          vi->pedal = true;
                        }
                      else
                        {
                          vi->state = Voice::STATE_RELEASE;
                          vi->env = 1.0;
                          need_reschedule = true;
                        }
                    }
                }
            }
          else if ((in_event.buffer[0] & 0xf0) == 0xb0)
            {
              //printf ("got midi controller event status=%x controller=%x value=%x\n",
              //        in_event.buffer[0], in_event.buffer[1], in_event.buffer[2]);
              if (in_event.buffer[1] == 0x40)
                {
                  pedal_down = in_event.buffer[2] > 0x40;
                  if (!pedal_down)
                    {
                      /* release voices which are sustained due to the pedal */
                      for (vector<Voice>::iterator vi = voices.begin(); vi != voices.end(); vi++)
                        {
                          if (vi->pedal && vi->state == Voice::STATE_ON)
                            {
                              vi->state = Voice::STATE_RELEASE;
                              vi->env = 1.0;
                              need_reschedule = true;
                            }
                        }
                    }
                }
            }


          // get next event
          event_index++;
          if (event_index < event_count)
            jack_midi_event_get (&in_event, port_buf, event_index);
        }
      if (need_reschedule)
        {
          reschedule();
          need_reschedule = false;
        }

      // compute boundary for processing
      size_t end;
      if (event_index < event_count)
        end = min (nframes, in_event.time);
      else
        end = nframes;


      // compute voices with state == STATE_ON
      for (vector<Voice*>::iterator avi = active_voices.begin(); avi != active_voices.end(); avi++)
        {
          Voice *v = *avi;
          for (int c = 0; c < channels; c++)
            {
              float samples[end - i];
              v->decoders[c]->process (end - i, NULL, NULL, samples);
              for (size_t j = i; j < end; j++)
                outputs[c][j] += samples[j-i] * v->velocity;
            }
        }
      // compute voices with state == STATE_RELEASE
      for (vector<Voice*>::iterator rvi = release_voices.begin(); rvi != release_voices.end(); rvi++)
        {
          Voice *v = *rvi;

          double v_decrement = (1000.0 / jack_mix_freq) / release_ms;
          size_t envelope_len = max (sm_round_positive (v->env / v_decrement), 0);
          size_t envelope_end = min (i + envelope_len, end);
          if (envelope_end < end)
            {
              v->state = Voice::STATE_IDLE;
              v->pedal = false;
              need_reschedule = true;
            }
          float envelope[envelope_end - i];
          for (size_t j = i; j < envelope_end; j++)
            {
              v->env -= v_decrement;
              envelope[j-i] = v->env;
            }
          for (int c = 0; c < channels; c++)
            {
              float samples[envelope_end - i];
              v->decoders[c]->process (envelope_end - i, NULL, NULL, samples);

              for (size_t j = i; j < envelope_end; j++)
                outputs[c][j] += samples[j - i] * envelope[j - i] * v->velocity;
            }
        }
      i = end;
    }
  return 0;
}

int
jack_process (jack_nframes_t nframes, void *arg)
{
  JackSynth *instance = reinterpret_cast<JackSynth *> (arg);
  return instance->process (nframes);
}

JackSynth::JackSynth()
{
  need_reschedule = false;
  pedal_down = false;
}

void
JackSynth::init (jack_client_t *client, WavSet *smset, int channels)
{
  this->smset = smset;
  this->channels = channels;
  release_ms = 150;
  voices.resize (64);
  for (vector<Voice>::iterator vi = voices.begin(); vi != voices.end(); vi++)
    {
      for (int c = 0; c < channels; c++)
        vi->decoders.push_back (new LiveDecoder (smset));
    }

  jack_set_process_callback (client, jack_process, this);

  jack_mix_freq = jack_get_sample_rate (client);

  // this might take a while, and cannot be used in RT callback
  voices[0].decoders[0]->precompute_tables (jack_mix_freq);

  input_port = jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

  const char *pattern;
  if (channels == 0)
    pattern = "audio_out";
  else
    pattern = "audio_out_%d";

  for (int c = 0; c < channels; c++)
    {
      string port_name = Birnet::string_printf (pattern, c + 1);
      output_ports.push_back (jack_port_register (client, port_name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));
    }

  if (jack_activate (client))
    {
      fprintf (stderr, "cannot activate client");
      exit (1);
    }
}

static bool
is_newline (char ch)
{
  return (ch == '\n' || ch == '\r');
}

class JackWindow : public Gtk::Window
{
  MorphPlanPtr    morph_plan;
  Gtk::VBox       vbox;
  Gtk::HBox       inst_hbox;
  Gtk::Label      inst_label;
  Gtk::Button     inst_button;
  MorphPlanWindow inst_window;
public:
  JackWindow (MorphPlanPtr plan) :
    morph_plan (plan),
    inst_window (morph_plan)
  {
    set_title ("SpectMorph JACK client");
    set_border_width (10);
    inst_label.set_text ("SpectMorph Instrument");
    inst_button.set_label ("Edit");
    inst_hbox.pack_start (inst_label);
    inst_hbox.pack_start (inst_button);
    inst_button.signal_clicked().connect (sigc::mem_fun (*this, &JackWindow::on_edit_clicked));
    vbox.pack_start (inst_hbox);
    add (vbox);
    show_all_children();
  }
  void
  on_edit_clicked()
  {
    if (inst_window.get_visible())
      inst_window.hide();
    else
      inst_window.show();
  }
};

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  Gtk::Main kit (argc, argv);

  if (argc > 2)
    {
      printf ("usage: smjack [ <plan_filename> ]\n");
      exit (1);
    }

  MorphPlanPtr morph_plan = new MorphPlan;

  if (argc == 2)
    {
      BseErrorType error;

      GenericIn *file = GenericIn::open (argv[1]);
      if (file)
        {
          error = morph_plan->load (file);
          delete file;
        }
      else
        {
          error = BSE_ERROR_FILE_NOT_FOUND;
        }
      if (error)
        {
          fprintf (stderr, "%s: can't open input file: %s: %s\n", argv[0], argv[1], bse_error_blurb (error));
          exit (1);
        }
    }

  JackWindow window (morph_plan);

  Gtk::Main::run (window);
#if 0

  printf ("loading %s ...", argv[1]);
  fflush (stdout);

  WavSet wset;
  BseErrorType error = wset.load (argv[1]);
  int n_channels = 1;
  for (size_t i = 0; i < wset.waves.size(); i++)
    n_channels = max (n_channels, wset.waves[i].channel + 1);
  printf ("%zd audio entries with %d channels found.\n", wset.waves.size(), n_channels);

  jack_client_t *client;
  client = jack_client_open ("smjack", JackNullOption, NULL);

  if (!client)
    {
      fprintf (stderr, "unable to connect to jack server\n");
      exit (1);
    }

  JackSynth synth;
  synth.init (client, &wset, n_channels);

  while (1)
    {
      printf ("SpectMorphJack> ");
      fflush (stdout);
      char buffer[1024];
      fgets (buffer, 1024, stdin);

      while (strlen (buffer) && is_newline (buffer[strlen (buffer) - 1]))
        buffer[strlen (buffer) - 1] = 0;

      if (strcmp (buffer, "q") == 0 || strcmp (buffer, "quit") == 0)
        {
          jack_deactivate (client);
          return 0;
        }
    }
#endif
}
