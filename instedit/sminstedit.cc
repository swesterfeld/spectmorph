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

class JackSynth : SignalReceiver
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
  void
  switch_to_sample (const Sample *sample, PlayMode play_mode, const Instrument *instrument = nullptr)
  {
    std::lock_guard<std::mutex> lg (synth_mutex);

    if (play_mode == PlayMode::SAMPLE)
      {
        WavSet wav_set;

        WavSetWave new_wave;
        new_wave.midi_note = sample->midi_note();
        // new_wave.path = "..";
        new_wave.channel = 0;
        new_wave.velocity_range_min = 0;
        new_wave.velocity_range_max = 127;

        Audio audio;
        audio.mix_freq = sample->wav_data.mix_freq();
        audio.fundamental_freq = note_to_freq (sample->midi_note());
        audio.original_samples = sample->wav_data.samples();
        new_wave.audio = audio.clone();

        wav_set.waves.push_back (new_wave);

        wav_set.save ("/tmp/midi_synth.smset");
        midi_synth->inst_edit_synth()->load_smset ("/tmp/midi_synth.smset", true);
      }
    else if (play_mode == PlayMode::REFERENCE)
      {
        Index index;
        index.load_file ("instruments:standard");

        string smset_dir = index.smset_dir();

        midi_synth->inst_edit_synth()->load_smset (smset_dir + "/synth-saw.smset", false);
      }
    else if (play_mode == PlayMode::SPECTMORPH)
      {
        WavSetBuilder *wbuilder = new WavSetBuilder (instrument);

        add_builder (wbuilder);
      }
  }
  int
  current_midi_note()
  {
    std::lock_guard<std::mutex> lg (synth_mutex);

    return m_current_midi_note;
  }
  WavSetBuilder *current_builder = nullptr;
  WavSetBuilder *next_builder = nullptr;
  void
  add_builder (WavSetBuilder *builder)
  {
    if (current_builder)
      {
        if (next_builder) /* kill and overwrite obsolete next builder */
          delete next_builder;

        next_builder = builder;
      }
    else
      {
        start_as_current (builder);
      }
  }
  void
  start_as_current (WavSetBuilder *builder)
  {
    current_builder = builder;
    new std::thread ([this] () {
      current_builder->run();

      finish_current_builder();
    });
  }
  void
  finish_current_builder()
  {
    std::lock_guard<std::mutex> lg (synth_mutex);

    WavSet wav_set;
    current_builder->get_result (wav_set);

    wav_set.save ("/tmp/midi_synth.smset");
    midi_synth->inst_edit_synth()->load_smset ("/tmp/midi_synth.smset", false);

    delete current_builder;
    current_builder = nullptr;

    if (next_builder)
      {
        WavSetBuilder *builder = next_builder;

        next_builder = nullptr;
        start_as_current (builder);
      }
  }
  bool
  have_builder()
  {
    std::lock_guard<std::mutex> lg (synth_mutex);

    return current_builder != nullptr;
  }
#endif
  void
  init (InstEditWindow *window)
  {
    connect (window->signal_inst_edit_update, this, &JackSynth::on_inst_edit_update);
  }

  void
  on_inst_edit_update (bool active, const string& filename, bool orig_samples)
  {
    std::lock_guard<std::mutex> lg (synth_mutex);
    if (active)
      midi_synth->inst_edit_synth()->load_smset (filename, orig_samples);
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

  InstEditWindow window (fn);
  jack_synth.init (&window);

  window.show();
  window.set_close_callback ([&]() { quit = true; });

  while (!quit)
    {
      window.wait_event_fps();
      window.process_events();
    }
  jack_deactivate (client);
}
