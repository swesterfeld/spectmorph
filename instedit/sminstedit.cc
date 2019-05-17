// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "spectmorphglui.hh"
#include "smpugixml.hh"
#include "sminstrument.hh"
#include "smsamplewidget.hh"
#include "smwavsetbuilder.hh"
#include "sminsteditwindow.hh"
#include "smeventloop.hh"
#include <thread>

#include <jack/jack.h>
#include <jack/midiport.h>

#include <stdio.h>
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::max;
using std::vector;
using std::map;

using pugi::xml_document;
using pugi::xml_node;

class JackSynth : public SignalReceiver
{
  jack_port_t *input_port;
  jack_port_t *output_port;

  Project project;
public:
  static int
  jack_process (jack_nframes_t nframes, void *arg)
  {
    JackSynth *instance = reinterpret_cast<JackSynth *> (arg);
    return instance->process (nframes);
  }

  int
  process (jack_nframes_t nframes)
  {
    project.try_update_synth();

    float     *audio_out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);
    void      *port_buf = jack_port_get_buffer (input_port, nframes);
    MidiSynth *midi_synth = project.midi_synth();

    jack_nframes_t event_count = jack_midi_get_event_count (port_buf);
    for (jack_nframes_t event_index = 0; event_index < event_count; event_index++)
      {
        jack_midi_event_t    in_event;
        jack_midi_event_get (&in_event, port_buf, event_index);

        midi_synth->add_midi_event (in_event.time, in_event.buffer);
      }
    midi_synth->process (audio_out, nframes);
    return 0;
  }

  JackSynth (jack_client_t *client)
  {
    project.set_mix_freq (jack_get_sample_rate (client));
    project.midi_synth()->set_inst_edit (true);

    jack_set_process_callback (client, jack_process, this);

    input_port = jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register (client, "audio_out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    if (jack_activate (client))
      {
        fprintf (stderr, "cannot activate client");
        exit (1);
      }
  }

  Project *
  get_project()
  {
    return &project;
  }
};

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  jack_client_t *client = jack_client_open ("sminstedit", JackNullOption, NULL);
  if (!client)
    {
      fprintf (stderr, "%s: unable to connect to jack server\n", argv[0]);
      exit (1);
    }

  JackSynth jack_synth (client);

  bool quit = false;

  Instrument instrument;
  if (argc > 1)
    {
      Error error = instrument.load (argv[1]);
      if (error)
        {
          fprintf (stderr, "%s: %s: %s.\n", argv[0], argv[1], error.message());
          return 1;
        }
    }

  EventLoop event_loop;
  InstEditWindow window (event_loop, &instrument, jack_synth.get_project()->synth_interface());

  window.show();
  window.set_close_callback ([&]() { quit = true; });

  while (!quit)
    {
      event_loop.wait_event_fps();
      event_loop.process_events();
    }
  jack_deactivate (client);
}
