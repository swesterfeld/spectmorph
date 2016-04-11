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

#include <jack/jack.h>
#include <jack/midiport.h>

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QCloseEvent>
#include <QSocketNotifier>

#include "smmain.hh"
#include "smjack.hh"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::max;
using std::min;

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
  // update plan with new parameters / new modules if necessary
  if (m_new_plan_mutex.tryLock())
    {
      if (m_new_plan)
        {
          morph_plan_synth->update_plan (m_new_plan);
          m_new_plan = NULL;
        }
      m_volume = m_new_volume;

      bool new_voices_active = false;
      for (size_t i = 0; i < voices.size(); i++)
        {
          if (voices[i].state != Voice::STATE_IDLE)
            {
              new_voices_active = true;
              break;
            }
        }
      if (m_voices_active != new_voices_active)
        {
          m_voices_active = new_voices_active;
          // wakeup main thread
          while (write (main_thread_wakeup_pfds[1], "W", 1) != 1)
            ;
        }
      m_new_plan_mutex.unlock();
    }

  float *control_in_1 = (jack_default_audio_sample_t *) jack_port_get_buffer (control_ports[0], nframes);
  float *control_in_2 = (jack_default_audio_sample_t *) jack_port_get_buffer (control_ports[1], nframes);
  float *audio_out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_ports[0], nframes);

  // zero output buffer, so voices can be added
  zero_float_block (nframes, audio_out);

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
              const int midi_note = in_event.buffer[1];
              const int midi_velocity = in_event.buffer[2];

              double velocity = midi_velocity / 127.0;
              // find unused voice
              vector<Voice>::iterator vi = voices.begin();
              while (vi != voices.end() && vi->state != Voice::STATE_IDLE)
                vi++;
              if (vi != voices.end())
                {
                  MorphOutputModule *output = vi->mp_voice->output();
                  if (output)
                    {
                      output->retrigger (0 /* channel */, freq_from_note (midi_note), midi_velocity);
                      vi->state = Voice::STATE_ON;
                      vi->midi_note = midi_note;
                      vi->velocity = velocity;
                      need_reschedule = true;
                    }
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
          MorphOutputModule *output = v->mp_voice->output();

          // update control input values
          v->mp_voice->set_control_input (0, control_in_1[i]);
          v->mp_voice->set_control_input (1, control_in_2[i]);

          if (output)
            {
              float samples[end - i];
              float *values[1] = { samples };
              output->process (end - i, values, 1);
              for (size_t j = i; j < end; j++)
                audio_out[j] += samples[j-i] * v->velocity;
            }
        }
      // compute voices with state == STATE_RELEASE
      for (vector<Voice*>::iterator rvi = release_voices.begin(); rvi != release_voices.end(); rvi++)
        {
          Voice *v = *rvi;

          // update control input values
          v->mp_voice->set_control_input (0, control_in_1[i]);
          v->mp_voice->set_control_input (1, control_in_2[i]);

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
          float samples[envelope_end - i];
          MorphOutputModule *output = v->mp_voice->output();
          if (output)
            {
              float *values[1] = { samples };
              output->process (envelope_end - i, values, 1);

              for (size_t j = i; j < envelope_end; j++)
                audio_out[j] += samples[j - i] * envelope[j - i] * v->velocity;
            }
        }
      i = end;
    }
  for (size_t i = 0; i < nframes; i++)
    audio_out[i] *= m_volume;

  morph_plan_synth->update_shared_state (nframes / jack_mix_freq * 1000);
  return 0;
}

int
jack_process (jack_nframes_t nframes, void *arg)
{
  JackSynth *instance = reinterpret_cast<JackSynth *> (arg);
  return instance->process (nframes);
}

JackSynth::JackSynth (jack_client_t *client) :
  m_voices_active (false)
{
  need_reschedule = false;
  pedal_down = false;
  m_volume = 1;
  m_new_volume = 1;
  morph_plan_synth = NULL;

  int pipe_rc = pipe (main_thread_wakeup_pfds);
  g_assert (pipe_rc == 0);

  QSocketNotifier *socket_notifier = new QSocketNotifier (main_thread_wakeup_pfds[0], QSocketNotifier::Read, this);
  connect (socket_notifier, SIGNAL (activated (int)), this, SLOT (on_voices_active_changed()));

  jack_mix_freq = jack_get_sample_rate (client);

  release_ms = 150;

  assert (morph_plan_synth == NULL);
  morph_plan_synth = new MorphPlanSynth (jack_mix_freq);

  voices.resize (64);
  for (vector<Voice>::iterator vi = voices.begin(); vi != voices.end(); vi++)
    vi->mp_voice = morph_plan_synth->add_voice();

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
  if (morph_plan_synth)
    {
      delete morph_plan_synth;
      morph_plan_synth = NULL;
    }
}

void
JackSynth::on_voices_active_changed()
{
  // clear wakeup pipe
  struct pollfd poll_fds[1];
  poll_fds[0].fd = main_thread_wakeup_pfds[0];
  poll_fds[0].events = POLLIN;
  poll_fds[0].revents = 0;

  if (poll (poll_fds, 1, 0) > 0)
    {
      char c;
      int rc = read (main_thread_wakeup_pfds[0], &c, 1);
      g_assert (rc != -1 || errno == EAGAIN);
    }

  Q_EMIT voices_active_changed();
}

void
JackSynth::preinit_plan (MorphPlanPtr plan)
{
  // this might take a while, and cannot be used in RT callback
  MorphPlanSynth mp_synth (jack_mix_freq);
  MorphPlanVoice *mp_voice = mp_synth.add_voice();
  mp_synth.update_plan (plan);

  MorphOutputModule *om = mp_voice->output();
  if (om)
    {
      om->retrigger (0, 440, 1);
      float s;
      float *values[1] = { &s };
      om->process (1, values, 1);
    }
}

void
JackSynth::change_plan (MorphPlanPtr plan)
{
  preinit_plan (plan);

  QMutexLocker locker (&m_new_plan_mutex);
  m_new_plan = plan;
}

void
JackSynth::change_volume (double new_volume)
{
  QMutexLocker locker (&m_new_plan_mutex);
  m_new_volume = new_volume;
}

bool
JackSynth::voices_active()
{
  QMutexLocker locker (&m_new_plan_mutex);
  return m_voices_active;
}

JackControlWidget::JackControlWidget (MorphPlanPtr plan, JackSynth *synth) :
  synth (synth),
  morph_plan (plan)
{
  QLabel *volume_label = new QLabel ("Volume", this);
  QSlider *volume_slider = new QSlider (Qt::Horizontal, this);
  volume_slider->setRange (-960, 240);
  volume_value_label = new QLabel (this);
  connect (volume_slider, SIGNAL (valueChanged(int)), this, SLOT (on_volume_changed(int)));
  volume_slider->setValue (-60);

  midi_led = new Led();
  midi_led->off();
  QHBoxLayout *hbox = new QHBoxLayout (this);
  hbox->addWidget (volume_label);
  hbox->addWidget (volume_slider);
  hbox->addWidget (volume_value_label);
  hbox->addWidget (midi_led);

  setLayout (hbox);
  setTitle ("Global Instrument Settings");

  connect (synth, SIGNAL (voices_active_changed()), this, SLOT (on_update_led()));
  connect (plan.c_ptr(), SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));

  on_plan_changed();
}

void
JackControlWidget::on_volume_changed (int new_volume)
{
  double new_volume_f = new_volume * 0.1;
  double new_decoder_volume = bse_db_to_factor (new_volume_f);
  volume_value_label->setText (string_locale_printf ("%.1f dB", new_volume_f).c_str());
  synth->change_volume (new_decoder_volume);
}

void
JackControlWidget::on_plan_changed()
{
  MorphPlanPtr plan_clone = new MorphPlan();

  vector<unsigned char> data;
  MemOut mo (&data);
  morph_plan->save (&mo);

  GenericIn *in = MMapIn::open_mem (&data[0], &data[data.size()]);
  plan_clone->load (in);
  delete in;

  synth->change_plan (plan_clone);
}

void
JackControlWidget::on_update_led()
{
  if (synth->voices_active())
    midi_led->on();
  else
    midi_led->off();
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  QApplication app (argc, argv);

  if (argc > 2)
    {
      printf ("usage: smjack [ <plan_filename> ]\n");
      exit (1);
    }

  MorphPlanPtr morph_plan = new MorphPlan;

  string filename;
  if (argc == 2)
    {
      Bse::Error error;

      GenericIn *file = GenericIn::open (argv[1]);
      if (file)
        {
          error = morph_plan->load (file);
          delete file;
        }
      else
        {
          error = Bse::Error::FILE_NOT_FOUND;
        }
      if (error != 0)
        {
          fprintf (stderr, "%s: can't open input file: %s: %s\n", argv[0], argv[1], bse_error_blurb (error));
          exit (1);
        }
      filename = argv[1];
    }
  else
    {
      MorphOperator *op = MorphOperator::create ("SpectMorph::MorphOutput", morph_plan.c_ptr());
      g_assert (op != NULL);
      morph_plan->add_operator (op);
    }

  jack_client_t *client = jack_client_open ("smjack", JackNullOption, NULL);
  if (!client)
    {
      fprintf (stderr, "%s: unable to connect to jack server\n", argv[0]);
      exit (1);
    }

  JackSynth synth (client);

  JackControlWidget *control_widget = new JackControlWidget (morph_plan, &synth);

  MorphPlanWindow window (morph_plan, "SpectMorph JACK Client");
  if (filename != "")
    window.set_filename (filename);
  window.add_control_widget (control_widget);
  window.show();
  int rc = app.exec();
  jack_deactivate (client);
  return rc;
}
