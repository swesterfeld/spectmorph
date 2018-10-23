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

class JackBackend : public Backend
{
  double jack_mix_freq;
  jack_port_t *input_port;
  jack_port_t *output_port;

  std::mutex decoder_mutex;
  std::unique_ptr<LiveDecoder> decoder;
  double decoder_factor = 0;
  int m_current_midi_note = -1;
  WavSet wav_set;
public:

  static int
  jack_process (jack_nframes_t nframes, void *arg)
  {
    JackBackend *instance = reinterpret_cast<JackBackend *> (arg);
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
    std::lock_guard<std::mutex> lg (decoder_mutex);
    float *audio_out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);

    void* port_buf = jack_port_get_buffer (input_port, nframes);
    jack_nframes_t event_count = jack_midi_get_event_count (port_buf);
    if (event_count)
      for (jack_nframes_t event_index = 0; event_index < event_count; event_index++)
      {
        jack_midi_event_t    in_event;
        jack_midi_event_get (&in_event, port_buf, event_index);

        if (in_event.buffer[0] == 0x90)
          {
            const int note = in_event.buffer[1];
            const double freq = note_to_freq (note);

            if (decoder)
              {
                decoder->retrigger (0, freq, 127, 48000);
                m_current_midi_note = note;
              }
            decoder_factor = 1;
          }
        if (in_event.buffer[0] == 0x80)
          {
            decoder_factor = 0;
            m_current_midi_note = -1;
          }
        //midi_synth->add_midi_event (in_event.time, in_event.buffer);
      }

    if (decoder)
      {
        decoder->process (nframes, nullptr, &audio_out[0]);
        for (uint i = 0; i < nframes; i++)
          audio_out[i] *= decoder_factor;
      }
    else
      {
        for (uint i = 0; i < nframes; i++)
          audio_out[i] = 0;
      }
    return 0;
  }

  JackBackend (jack_client_t *client)
  {
    jack_mix_freq = jack_get_sample_rate (client);

    jack_set_process_callback (client, jack_process, this);

    input_port = jack_port_register (client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register (client, "audio_out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    if (jack_activate (client))
      {
        fprintf (stderr, "cannot activate client");
        exit (1);
      }
  }

  void
  switch_to_sample (const Sample *sample, PlayMode play_mode, const Instrument *instrument = nullptr)
  {
    std::lock_guard<std::mutex> lg (decoder_mutex);

    if (play_mode == PlayMode::SAMPLE)
      {
        wav_set.clear();

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

        decoder.reset (new LiveDecoder (&wav_set));
        decoder->enable_original_samples (true);
      }
    else if (play_mode == PlayMode::REFERENCE)
      {
        Index index;
        index.load_file ("instruments:standard");

        string smset_dir = index.smset_dir();

        decoder.reset (new LiveDecoder (WavSetRepo::the()->get (smset_dir + "/synth-saw.smset")));
      }
    else if (play_mode == PlayMode::SPECTMORPH)
      {
        WavSetBuilder *wbuilder = new WavSetBuilder (instrument);

        add_builder (wbuilder);
      }
    else
      {
        decoder.reset (nullptr); // not yet implemented
      }
  }
  int
  current_midi_note()
  {
    std::lock_guard<std::mutex> lg (decoder_mutex);

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
    std::lock_guard<std::mutex> lg (decoder_mutex);

    current_builder->get_result (wav_set);

    decoder.reset (new LiveDecoder (&wav_set));

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
    std::lock_guard<std::mutex> lg (decoder_mutex);

    return current_builder != nullptr;
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

  JackBackend jack_backend (client);

  bool quit = false;

  string fn = (argc > 1) ? argv[1] : "test.sminst";
  InstEditWindow window (fn, &jack_backend);

  window.show();
  window.set_close_callback ([&]() { quit = true; });

  while (!quit)
    {
      window.wait_event_fps();
      window.process_events();
      window.update_led();
    }
  jack_deactivate (client);
}
