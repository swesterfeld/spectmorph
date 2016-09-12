// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_JACK_HH
#define SPECTMORPH_JACK_HH

#include "smmorphplanwindow.hh"
#include "smmorphplansynth.hh"
#include "smmorphplanvoice.hh"
#include "smmidisynth.hh"
#include "smled.hh"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QGroupBox>

#include <jack/jack.h>

namespace SpectMorph
{

class JackSynth : public QObject
{
  Q_OBJECT
protected:
  double                        jack_mix_freq;
  jack_port_t                  *input_port;
  std::vector<jack_port_t *>    output_ports;
  std::vector<jack_port_t *>    control_ports;

  double                        m_volume;

  MidiSynth                    *midi_synth;

  QMutex                        m_new_plan_mutex;
  MorphPlanPtr                  m_new_plan;
  double                        m_new_volume;
  bool                          m_voices_active;

  int                           main_thread_wakeup_pfds[2];

public:
  JackSynth (jack_client_t *client);
  ~JackSynth();

  void preinit_plan (MorphPlanPtr plan);
  void change_plan (MorphPlanPtr plan);
  void change_volume (double new_volume);
  bool voices_active();
  int  process (jack_nframes_t nframes);

public slots:
  void on_voices_active_changed();

signals:
  void voices_active_changed();
};

class JackControlWidget : public QGroupBox
{
  Q_OBJECT

  JackSynth    *synth;
  MorphPlanPtr  morph_plan;
  QLabel       *volume_value_label;
  Led          *midi_led;
  QLabel       *inst_status;

public:
  JackControlWidget (MorphPlanPtr plan, JackSynth *synth);

public slots:
  void on_plan_changed();
  void on_index_changed();
  void on_volume_changed (int new_volume);
  void on_update_led();
};

}

#endif
