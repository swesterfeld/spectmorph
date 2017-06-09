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
#include "spectmorph.hh"

#include <bse/bsemathsignal.hh>
#include <bse/bseengine.hh>

#include <stdio.h>

using std::string;
using std::map;
using std::vector;

namespace SpectMorph {

using namespace Bse;

class Osc : public OscBase {
  struct Properties : public OscProperties {
    Osc *osc;
    Properties (Osc *osc) :
      OscProperties (osc),
      osc (osc)
    {
    }
  };
  class Module : public SynthesisModule {
  private:
    MorphPlanVoice *morph_plan_voice;
    MorphPlanSynth *morph_plan_synth;
    Osc            *osc;
    float           last_sync_level;
    float           current_freq;
    bool            need_retrigger;
    float           frequency;
  public:
    Module() :
      morph_plan_voice (NULL),
      morph_plan_synth (NULL),
      osc (NULL),
      need_retrigger (false)
    {
      //
    }
    void reset()
    {
      need_retrigger = true;
    }
    void
    process (unsigned int n_values)
    {
      QMutexLocker lock (&osc->morph_plan_synth_mutex());

      //const gfloat *sync_in = istream (ICHANNEL_AUDIO_OUT).values;
      if (istream (ICHANNEL_CTRL_IN1).connected)
        {
          const gfloat *ctrl_in = istream (ICHANNEL_CTRL_IN1).values;
          morph_plan_voice->set_control_input (0, ctrl_in[0]);
        }
      else
        {
          morph_plan_voice->set_control_input (0, 0);
        }
      if (istream (ICHANNEL_CTRL_IN2).connected)
        {
          const gfloat *ctrl_in = istream (ICHANNEL_CTRL_IN2).values;
          morph_plan_voice->set_control_input (1, ctrl_in[0]);
        }
      else
        {
          morph_plan_voice->set_control_input (1, 0);
        }
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
      if (!morph_plan_voice->output())
        {
          ostream_set (OCHANNEL_AUDIO_OUT1, const_values (0));
        }
      else
        {
          /* we only allow one output channel (although we could do more),
           * but we want to match the behaviour of the VST|LV2 plugins
           * (which currently are mono)
           */
          float *audio_out[4] = { NULL, NULL, NULL, NULL };

          if (ostream (OCHANNEL_AUDIO_OUT1).connected)
            audio_out[0] = ostream (OCHANNEL_AUDIO_OUT1).values;

          if (istream (ICHANNEL_FREQ_IN).connected)
            {
              const gfloat *freq_in = istream (ICHANNEL_FREQ_IN).values;
              float freq_in_converted[n_values];

              for (unsigned int i = 0; i < n_values; i++)
                freq_in_converted[i] = BSE_SIGNAL_TO_FREQ (freq_in[i]);

              morph_plan_voice->output()->process (n_values, audio_out, 4, freq_in_converted);
            }
          else
            {
              morph_plan_voice->output()->process (n_values, audio_out, 4);
            }
        }
      osc->update_shared_state (tick_stamp(), mix_freq());
    }
    void
    retrigger (float freq, int midi_velocity)
    {
      if (morph_plan_voice->output())
        morph_plan_voice->output()->retrigger (0, freq, midi_velocity);

      current_freq = freq;
    }
    void
    config (Properties *properties)
    {
      frequency = properties->frequency;
      osc       = properties->osc;

      if (!morph_plan_voice)
        {
          morph_plan_synth = osc->morph_plan_synth (mix_freq());
          morph_plan_voice = morph_plan_synth->add_voice();
        }

      QMutexLocker lock (&osc->morph_plan_synth_mutex());
      MorphPlanPtr plan = osc->take_new_morph_plan();
      if (plan)
        morph_plan_synth->update_plan (plan);
    }
  };

  FILE    *gui_pipe_stdin;
  FILE    *gui_pipe_stdout;
  GPollFD  gui_poll_fd;
  GSource *gui_source;
  int      gui_pid;

  MorphPlanPtr     m_morph_plan;
  MorphPlanPtr     m_new_morph_plan;
  MorphPlanSynth  *m_morph_plan_synth;
  uint64           last_tick_stamp;
  QMutex           m_morph_plan_synth_mutex;

public:
  MorphPlanPtr
  take_new_morph_plan()
  {
    MorphPlanPtr plan = m_new_morph_plan;
    m_new_morph_plan = NULL;
    return plan;
  }
  QMutex&
  morph_plan_synth_mutex()
  {
    return m_morph_plan_synth_mutex;
  }
  MorphPlanSynth *
  morph_plan_synth (float mix_freq)
  {
    QMutexLocker lock (&m_morph_plan_synth_mutex);

    if (!m_morph_plan_synth)
      m_morph_plan_synth = new MorphPlanSynth (mix_freq);

    double epsilon = 1e-8;
    if (fabs (m_morph_plan_synth->mix_freq() - mix_freq) > epsilon)
      {
        // mix_freq changed
        delete m_morph_plan_synth;
        m_morph_plan_synth = new MorphPlanSynth (mix_freq);
      }
    return m_morph_plan_synth;
  }
  void
  update_shared_state (uint64 tick_stamp, float mix_freq)
  {
    if (tick_stamp > last_tick_stamp)
      {
        /* the problem with bsetool render2wav is that it produces lots of silence before
         * the actual song - therefore we skip the first time update_shared_state() would
         * be called with the duration of that silence
         */
        if (last_tick_stamp)
          {
            double delta_time_ms = (tick_stamp - last_tick_stamp) / mix_freq * 1000;
            m_morph_plan_synth->update_shared_state (delta_time_ms);
          }
        last_tick_stamp = tick_stamp;
      }
  }
  void
  prepare1()
  {
    m_new_morph_plan = m_morph_plan;
  }
  void
  reset1()
  {
    if (m_morph_plan_synth)
      {
        delete m_morph_plan_synth;
        m_morph_plan_synth = NULL;
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
      }
  }
  bool
  property_changed (OscPropertyID prop_id)
  {
    switch (prop_id)
      {
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
        case PROP_PLAN:
          m_morph_plan = new MorphPlan();
          m_morph_plan->set_plan_str (plan.c_str());
            {
              QMutexLocker lock (&m_morph_plan_synth_mutex);
              m_new_morph_plan = m_morph_plan;
            }
#if 0
          printf ("==<>== MorphPlan updated: new plan has %d chars; %zd operators\n",
                  plan.length(), m_morph_plan->operators().size());
#endif
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
    m_morph_plan_synth = NULL;
    last_tick_stamp = 0;

    static bool sm_init_ok = false;
    if (!sm_init_ok)
      {
        sm_init_plugin();
        sm_init_ok = true;
      }
  }
  void
  start_gui()
  {
    int child_stdin = -1, child_stdout = -1;
    char **argv;
    argv = (char **) g_malloc (3 * sizeof (char *));
    argv[0] = (char *) "spectmorphoscgui";
    argv[1] = BSE_OBJECT_UNAME (gobject());
    argv[2] = NULL;
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
