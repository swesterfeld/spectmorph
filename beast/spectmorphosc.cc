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

#include "spectmorphosc.genidl.hh"
#include "smaudio.hh"
#include "smlivedecoder.hh"
#include "smwavset.hh"
#include "smmain.hh"

#include <bse/bsemathsignal.h>
#include <bse/bseengine.h>

#include <stdio.h>

using std::string;
using std::map;
using std::vector;

namespace SpectMorph {

using namespace Bse;

class AudioRepo {
  Birnet::Mutex mutex;
  map<string, WavSet *> wav_set_map;
public:
  WavSet *get_wav_set (const string& filename)
  {
    Birnet::AutoLocker lock (mutex);

    return wav_set_map[filename];
  }
  void put_wav_set (const string& filename, WavSet *wav_set)
  {
    Birnet::AutoLocker lock (mutex);

    wav_set_map[filename] = wav_set;
  }
} audio_repo;

class Osc : public OscBase {
  struct Properties : public OscProperties {
    Properties (Osc *osc) : OscProperties (osc)
    {
    }
  };
  class Module : public SynthesisModule {
  private:
    WavSet        *wav_set;
    LiveDecoder   *live_decoder;
    float          last_sync_level;
    float          current_freq;
    bool           need_retrigger;
    int            channel;
    float          frequency;
  public:
    Module() :
      wav_set (NULL),
      live_decoder (NULL),
      need_retrigger (false)
    {
      //
    }
    ~Module()
    {
      if (live_decoder)
        {
          delete live_decoder;
          live_decoder = NULL;
        }
    }
    void reset()
    {
      need_retrigger = true;
    }
    void process (unsigned int n_values)
    {
      //const gfloat *sync_in = istream (ICHANNEL_AUDIO_OUT).values;
      gfloat *audio_out = ostream (OCHANNEL_AUDIO_OUT).values;
      if (need_retrigger)
        {
          // get frequency
          const gfloat *freq_in = istream (ICHANNEL_FREQ_IN).values;
          float new_freq = istream (ICHANNEL_FREQ_IN).connected ? BSE_SIGNAL_TO_FREQ (freq_in[0]) : frequency;

          // get velocity
          const gfloat *velocity_in = istream (ICHANNEL_VELOCITY_IN).values;
          float new_velocity = istream (ICHANNEL_VELOCITY_IN).connected ? velocity_in[0] : 1.0;
          int   midi_velocity = CLAMP (sm_round_positive (new_velocity * 127), 0, 127);

          retrigger (new_freq, midi_velocity);
          need_retrigger = false;
        }
      live_decoder->process (n_values, NULL, NULL, audio_out);
    }
    void
    retrigger (float freq, int midi_velocity)
    {
      if (live_decoder)
        live_decoder->retrigger (channel, freq, midi_velocity, mix_freq());

      current_freq = freq;
    }
    void
    config (Properties *properties)
    {
      wav_set = audio_repo.get_wav_set (properties->filename.c_str());
      channel = properties->channel - 1;
      frequency = properties->frequency;

      if (live_decoder)
        delete live_decoder;

      live_decoder = new LiveDecoder (wav_set);
      live_decoder->enable_sines (properties->sines);
      live_decoder->enable_noise (properties->noise);
      live_decoder->enable_original_samples (properties->use_samples);
    }
  };

  FILE    *gui_pipe_stdin;
  FILE    *gui_pipe_stdout;
  GPollFD  gui_poll_fd;
  GSource *gui_source;
  int      gui_pid;
public:
  void
  load_file (const string& filename)
  {
    static bool sm_init_ok = false;
    if (!sm_init_ok)
      {
        sm_init_plugin();
        sm_init_ok = true;
      }
    BseErrorType error;

    WavSet *wav_set = new WavSet;
    error = wav_set->load (filename);
    if (!error)
      {
        audio_repo.put_wav_set (filename, wav_set);
      }
    else
      delete wav_set;
  }
  void
  prepare1()
  {
    WavSet *wav_set = audio_repo.get_wav_set (filename.c_str());
    if (wav_set)
      {
        LiveDecoder decoder (wav_set);
        decoder.precompute_tables (bse_engine_sample_freq());
      }
  }
  static gboolean
  gui_source_pending (Osc *osc, gint *timeout)
  {
    *timeout = -1;
    return osc->gui_poll_fd.revents & G_IO_IN;
  }
  static void
  gui_source_dispatch (Osc *osc)
  {
    string s;
    int ch;
    while ((ch = fgetc (osc->gui_pipe_stdout)) > 0)
      {
        if (ch == '\n')
          break;
        s += (char) ch;
      }
    if (s.substr (0, 3) == "pid")
      {
        osc->gui_pid = atoi (s.substr (4).c_str());
      }
    else if (s == "quit")
      {
        osc->stop_gui();
      }
    else
      {
        osc->set ("plan", s.c_str(), NULL);
        osc->notify ("plan");
        printf ("PLAN: %s\n", s.c_str());
      }
  }
  bool
  property_changed (OscPropertyID prop_id)
  {
    switch (prop_id)
      {
        case PROP_FILENAME:
          load_file (filename.c_str());
          break;
        case PROP_EDIT_SETTINGS:
          if (edit_settings)
            {
              if (gui_pipe_stdin)
                {
                  stop_gui();
                }
              else
                {
                  start_gui();
                }
            }
          edit_settings = false;
          break;
        default:
          break;
      }
    return false;
  }
  Osc()
  {
    gui_pipe_stdin = NULL;
    gui_pipe_stdout = NULL;
  }
  void
  start_gui()
  {
    int child_stdin = -1, child_stdout = -1;
    char **argv;
    argv = (char **) g_malloc (2 * sizeof (char *));
    argv[0] = "spectmorphoscgui";
    argv[1] = NULL;
    GError *error = NULL;
    GPid child_pid;
    g_spawn_async_with_pipes (NULL, /* working directory = current dir */
                              argv, /* arguments */
                              NULL, /* inherit environment */
                              G_SPAWN_SEARCH_PATH,
                              NULL, NULL, /* no child setup */
                              &child_pid,
                              &child_stdin,
                              &child_stdout,
                              NULL, /* inherid stderr */
                              &error);
    g_free (argv);
    gui_pipe_stdin = fdopen (child_stdin, "w");
    gui_pipe_stdout = fdopen (child_stdout, "r");
    gui_poll_fd.fd = fileno (gui_pipe_stdout);
    gui_poll_fd.events = G_IO_IN;
    gui_source = g_source_simple (G_PRIORITY_LOW,
                              (GSourcePending)  gui_source_pending,
                              (GSourceDispatch) gui_source_dispatch,
                              this,
                              NULL,
                              &gui_poll_fd,
                              NULL);
    g_source_attach (gui_source, NULL);
    g_source_unref (gui_source);

    // set initial plan
    fprintf (gui_pipe_stdin, "%s\n", plan.c_str());
    fflush (gui_pipe_stdin);
  }
  void
  stop_gui()
  {
    if (gui_pipe_stdout)
      {
        kill (gui_pid, SIGTERM);
        fclose (gui_pipe_stdin);
        fclose (gui_pipe_stdout);
        g_source_destroy (gui_source);
        gui_pipe_stdin  = NULL;
        gui_pipe_stdout = NULL;
        gui_source      = NULL;
      }
  }
  ~Osc()
  {
    stop_gui();
  }
  /* implement creation and config methods for synthesis Module */
  BSE_EFFECT_INTEGRATE_MODULE (Osc, Module, Properties);
};

BSE_CXX_DEFINE_EXPORTS();
BSE_CXX_REGISTER_EFFECT (Osc);

}
