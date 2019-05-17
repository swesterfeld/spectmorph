// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplan.hh"
#include "smmorphplanview.hh"
#include "smmorphplanwindow.hh"
#include "smmorphplanvoice.hh"
#include "smmorphplansynth.hh"
#include "smmorphoutputmodule.hh"
#include "smmemout.hh"
#include "smled.hh"
#include "smutils.hh"
#include "smeventloop.hh"

#include <jack/jack.h>
#include <jack/midiport.h>

#include "smmain.hh"
#include "smjack.hh"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::max;
using std::min;

int
JackSynth::process (jack_nframes_t nframes)
{
  m_project->try_update_synth();

  const float *control_in_1 = (jack_default_audio_sample_t *) jack_port_get_buffer (control_ports[0], nframes);
  const float *control_in_2 = (jack_default_audio_sample_t *) jack_port_get_buffer (control_ports[1], nframes);
  float       *audio_out    = (jack_default_audio_sample_t *) jack_port_get_buffer (output_ports[0], nframes);

  MidiSynth   *midi_synth   = m_project->midi_synth();

  void* port_buf = jack_port_get_buffer (input_port, nframes);
  jack_nframes_t event_count = jack_midi_get_event_count (port_buf);

  for (jack_nframes_t event_index = 0; event_index < event_count; event_index++)
    {
      jack_midi_event_t    in_event;
      jack_midi_event_get (&in_event, port_buf, event_index);

      midi_synth->add_midi_event (in_event.time, in_event.buffer);
    }

  // update control input values
  midi_synth->set_control_input (0, control_in_1[0]);
  midi_synth->set_control_input (1, control_in_2[0]);

  midi_synth->process (audio_out, nframes);

  return 0;
}

int
jack_process (jack_nframes_t nframes, void *arg)
{
  JackSynth *instance = reinterpret_cast<JackSynth *> (arg);
  return instance->process (nframes);
}

JackSynth::JackSynth (jack_client_t *client, Project *project) :
  m_project (project)
{
  m_project->set_mix_freq (jack_get_sample_rate (client));

  jack_set_process_callback (client, jack_process, this);

  input_port = jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
  control_ports.push_back (jack_port_register (client, "control_in_1", JACK_DEFAULT_AUDIO_TYPE,
                                               JackPortIsInput, 0));
  control_ports.push_back (jack_port_register (client, "control_in_2", JACK_DEFAULT_AUDIO_TYPE,
                                               JackPortIsInput, 0));
  output_ports.push_back (jack_port_register (client, "audio_out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));

  if (jack_activate (client))
    {
      fprintf (stderr, "cannot activate client");
      exit (1);
    }
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  if (argc > 2)
    {
      printf ("usage: smjack [ <plan_filename> ]\n");
      exit (1);
    }

  Project project;

  jack_client_t *client = jack_client_open ("smjack", JackNullOption, NULL);
  if (!client)
    {
      fprintf (stderr, "%s: unable to connect to jack server\n", argv[0]);
      exit (1);
    }

  JackSynth   synth (client, &project);

  string filename;
  if (argc == 2)
    {
      Error error = project.load (argv[1]);

      if (error)
        {
          fprintf (stderr, "%s: can't open input file: %s: %s\n", argv[0], argv[1], error.message());
          exit (1);
        }
      filename = argv[1];
    }

  EventLoop event_loop;
  MorphPlanWindow window (event_loop, "SpectMorph JACK Client", /* win_id */ 0, /* resize */ false, project.morph_plan());
  if (filename != "")
    window.set_filename (filename);
  window.show();
  bool quit = false;

  window.set_close_callback ([&]() { quit = true; });

  while (!quit)
    {
      event_loop.wait_event_fps();
      event_loop.process_events();
    }
  jack_deactivate (client);
  return 0;
}
