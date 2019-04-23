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

  // update plan with new parameters / new modules if necessary
  if (m_project->synth_mutex().try_lock())
    {
      m_volume = m_new_volume;

      m_voices_active = midi_synth->active_voice_count() > 0;

      m_project->synth_mutex().unlock();
    }

  const float *control_in_1 = (jack_default_audio_sample_t *) jack_port_get_buffer (control_ports[0], nframes);
  const float *control_in_2 = (jack_default_audio_sample_t *) jack_port_get_buffer (control_ports[1], nframes);
  float       *audio_out    = (jack_default_audio_sample_t *) jack_port_get_buffer (output_ports[0], nframes);

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

  for (size_t i = 0; i < nframes; i++)
    audio_out[i] *= m_volume;

  return 0;
}

int
jack_process (jack_nframes_t nframes, void *arg)
{
  JackSynth *instance = reinterpret_cast<JackSynth *> (arg);
  return instance->process (nframes);
}

JackSynth::JackSynth (jack_client_t *client, Project *project) :
  m_project (project),
  m_voices_active (false)
{
  m_volume = 1;
  m_new_volume = 1;

  jack_mix_freq = jack_get_sample_rate (client);

  midi_synth = new MidiSynth (jack_mix_freq, 64);
  m_project->change_midi_synth (midi_synth);

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

JackSynth::~JackSynth()
{
  if (midi_synth)
    {
      delete midi_synth;
      midi_synth = NULL;
    }
}

void
JackSynth::change_plan (MorphPlanPtr plan)
{
  m_project->update_plan (plan);
}

void
JackSynth::change_volume (double new_volume)
{
  std::lock_guard<std::mutex> lg (m_project->synth_mutex());
  m_new_volume = new_volume;
}

bool
JackSynth::voices_active()
{
  std::lock_guard<std::mutex> lg (m_project->synth_mutex());
  return m_voices_active;
}

JackControl::JackControl (MorphPlanPtr plan, MorphPlanWindow& window, MorphPlanControl *control_widget, JackSynth *synth) :
  synth (synth),
  morph_plan (plan)
{
  m_control_widget = control_widget;
  m_control_widget->set_volume (-6); // default volume
  on_volume_changed (-6);

  connect (m_control_widget->signal_volume_changed, this, &JackControl::on_volume_changed);
  connect (plan->signal_plan_changed, this, &JackControl::on_plan_changed);

  on_plan_changed();
}

void
JackControl::update_led()
{
  m_control_widget->set_led (synth->voices_active());
}

void
JackControl::on_volume_changed (double new_volume)
{
  double new_decoder_volume = db_to_factor (new_volume);
  synth->change_volume (new_decoder_volume);
}

void
JackControl::on_plan_changed()
{
  synth->change_plan (morph_plan->clone());
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
  MorphPlanPtr morph_plan = new MorphPlan (project);

  string filename;
  if (argc == 2)
    {
      Error error;

      GenericIn *file = GenericIn::open (argv[1]);
      if (file)
        {
          error = morph_plan->load (file);
          delete file;
        }
      else
        {
          error = Error::FILE_NOT_FOUND;
        }
      if (error != 0)
        {
          fprintf (stderr, "%s: can't open input file: %s: %s\n", argv[0], argv[1], sm_error_blurb (error));
          exit (1);
        }
      filename = argv[1];
    }
  else
    {
      morph_plan->load_default();
    }

  jack_client_t *client = jack_client_open ("smjack", JackNullOption, NULL);
  if (!client)
    {
      fprintf (stderr, "%s: unable to connect to jack server\n", argv[0]);
      exit (1);
    }

  JackSynth   synth (client, &project);

  EventLoop event_loop;
  MorphPlanWindow window (event_loop, "SpectMorph JACK Client", /* win_id */ 0, /* resize */ false, morph_plan);
  if (filename != "")
    window.set_filename (filename);
  JackControl control (morph_plan, window, window.control_widget(), &synth);
  window.show();
  bool quit = false;

  window.set_close_callback ([&]() { quit = true; });

  while (!quit)
    {
      event_loop.wait_event_fps();
      control.update_led();
      event_loop.process_events();
    }
  jack_deactivate (client);
  return 0;
}
