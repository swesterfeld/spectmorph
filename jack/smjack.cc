// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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

// #define SM_MALLOC_TRACER 1
// #define SM_MALLOC_TRACER_BACKTRACE 1
// #define SM_MALLOC_TRACER_GROUP 1
#include "smmalloctracer.hh"

static constexpr bool TRACE_PROCESS_TIME = false;

using namespace SpectMorph;

using std::vector;
using std::string;
using std::max;
using std::min;

int
JackSynth::process (jack_nframes_t n_frames) noexcept
{
  double t;
  if (TRACE_PROCESS_TIME)
    t = get_time();

  m_project->try_update_synth();

  float       *audio_out_left   = (jack_default_audio_sample_t *) jack_port_get_buffer (output_ports[0], n_frames);
  float       *audio_out_right  = (jack_default_audio_sample_t *) jack_port_get_buffer (output_ports[1], n_frames);

  MidiSynth   *midi_synth   = m_project->midi_synth();

  jack_position_t pos;
  const bool rolling = (jack_transport_query (client, &pos) == JackTransportRolling);
  if (pos.valid & JackPositionBBT)
    {
      /* use tempo from jack transport (if available) */
      midi_synth->set_tempo (pos.beats_per_minute);

      /* we try to synchronize the ppq position with the values available from JACK
       * unfortunately, this doesn't work if beats_per_bar changes (i.e. time signature changes 4/4 -> 3/4)
       *
       * in principle it would be better to use pos.bar_start_tick, but Ardour doesn't set this
       */
      if (rolling)
        {
          double ppq_pos = ((pos.bar - 1) * pos.beats_per_bar) + (pos.beat - 1) + pos.tick / pos.ticks_per_beat;
          // double ppq_pos2 = (pos.bar_start_tick + pos.tick) / pos.ticks_per_beat + (pos.beat - 1);
          ppq_pos *= 4 / pos.beat_type; // convert position to quarter notes
          midi_synth->set_ppq_pos (ppq_pos);
        }
    }

  void* port_buf = jack_port_get_buffer (input_port, n_frames);
  jack_nframes_t event_count = jack_midi_get_event_count (port_buf);

  for (jack_nframes_t event_index = 0; event_index < event_count; event_index++)
    {
      jack_midi_event_t    in_event;
      jack_midi_event_get (&in_event, port_buf, event_index);

      midi_synth->add_midi_event (in_event.time, in_event.buffer);
    }

  midi_synth->process (audio_out_left, n_frames);

  // proper stereo support will be added later
  std::copy (audio_out_left, audio_out_left + n_frames, audio_out_right);

  if (TRACE_PROCESS_TIME)
    sm_printf ("%f %f\n", (get_time() - t) * 1000, n_frames * 1000. / m_mix_freq);
  return 0;
}

int
jack_process (jack_nframes_t n_frames, void *arg)
{
  static bool first = true;
  if (first)
    {
      sm_set_dsp_thread();
      first = false;
    }

  JackSynth *instance = reinterpret_cast<JackSynth *> (arg);
  return instance->process (n_frames);
}

JackSynth::JackSynth (jack_client_t *client, Project *project) :
  client (client),
  m_project (project)
{
  m_mix_freq = jack_get_sample_rate (client);
  m_project->set_mix_freq (m_mix_freq);

  // JACK version of SpectMorph exports its control signal by CC#16,...
  m_project->midi_synth()->set_control_by_cc (true);

  jack_set_process_callback (client, jack_process, this);

  input_port = jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
  output_ports.push_back (jack_port_register (client, "audio_out_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));
  output_ports.push_back (jack_port_register (client, "audio_out_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));

  if (jack_activate (client))
    {
      fprintf (stderr, "cannot activate client");
      exit (1);
    }
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  sm_set_ui_thread();

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

  MallocTracer malloc_tracer;
  while (!quit)
    {
      event_loop.wait_event_fps();
      event_loop.process_events();
      malloc_tracer.print_stats();
    }
  jack_client_close (client);
  return 0;
}
