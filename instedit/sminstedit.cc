// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "spectmorphglui.hh"
#include "smpugixml.hh"
#include "sminstrument.hh"
#include "smsamplewidget.hh"
#include "smwavsetbuilder.hh"
#include "sminsteditwindow.hh"
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

class JackSynth : public SignalReceiver,
                  public SynthInterface
{
  double jack_mix_freq;
  jack_port_t *input_port;
  jack_port_t *output_port;

  std::mutex synth_mutex;
  int m_current_midi_note = -1;
  std::unique_ptr<MidiSynth> midi_synth;
public:

  static int
  jack_process (jack_nframes_t nframes, void *arg)
  {
    JackSynth *instance = reinterpret_cast<JackSynth *> (arg);
    return instance->process (nframes);
  }

  double
  note_to_freq (int note)
  {
    return 440 * exp (log (2) * (note - 69) / 12.0);
  }
  int
  process (jack_nframes_t nframes)
  {
    std::lock_guard<std::mutex> lg (synth_mutex);

    float *audio_out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);
    void *port_buf = jack_port_get_buffer (input_port, nframes);

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
    jack_mix_freq = jack_get_sample_rate (client);

    midi_synth.reset (new MidiSynth (jack_mix_freq, 64));
    midi_synth->set_inst_edit (true);

    jack_set_process_callback (client, jack_process, this);

    input_port = jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register (client, "audio_out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    if (jack_activate (client))
      {
        fprintf (stderr, "cannot activate client");
        exit (1);
      }
  }
#if 0
  int
  current_midi_note()
  {
    std::lock_guard<std::mutex> lg (synth_mutex);

    return m_current_midi_note;
  }
#endif

  void
  synth_inst_edit_update (bool active, const string& filename, bool orig_samples)
  {
    InstEditUpdate ie_update (active, filename, orig_samples);
    ie_update.prepare();

    std::lock_guard<std::mutex> lg (synth_mutex);
    ie_update.run_rt (midi_synth.get());
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

  string fn = (argc > 1) ? argv[1] : "test.sminst";

  InstEditWindow window (fn, &jack_synth);

  window.show();
  window.set_close_callback ([&]() { quit = true; });

  while (!quit)
    {
      window.wait_event_fps();
      window.process_events();
    }
  jack_deactivate (client);
}
