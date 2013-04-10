// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplanwindow.hh"
#include "smmorphplansynth.hh"
#include "smmorphplanvoice.hh"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QGroupBox>

#include <jack/jack.h>

namespace SpectMorph
{

class Voice
{
public:
  enum State {
    STATE_IDLE,
    STATE_ON,
    STATE_RELEASE
  };
  MorphPlanVoice *mp_voice;

  State        state;
  bool         pedal;
  int          midi_note;
  double       env;
  double       velocity;

  Voice() :
    mp_voice (NULL),
    state (STATE_IDLE),
    pedal (false)
  {
  }
  ~Voice()
  {
    mp_voice = NULL;
  }
};

class JackSynth
{
protected:
  double                        jack_mix_freq;
  jack_port_t                  *input_port;
  std::vector<jack_port_t *>    output_ports;
  std::vector<jack_port_t *>    control_ports;
  bool                          need_reschedule;
  bool                          pedal_down;

  double                        release_ms;
  double                        m_volume;

  MorphPlanSynth               *morph_plan_synth;
  std::vector<Voice>            voices;
  std::vector<Voice*>           active_voices;
  std::vector<Voice*>           release_voices;

  QMutex                        m_new_plan_mutex;
  MorphPlanPtr                  m_new_plan;
  double                        m_new_volume;

public:
  JackSynth();
  ~JackSynth();

  void init (jack_client_t *client, MorphPlanPtr morph_plan);
  void preinit_plan (MorphPlanPtr plan);
  void change_plan (MorphPlanPtr plan);
  void change_volume (double new_volume);
  int  process (jack_nframes_t nframes);
  void reschedule();
};

class JackControlWidget : public QGroupBox
{
  Q_OBJECT

  JackSynth    *synth;
  MorphPlanPtr  morph_plan;
  QLabel       *volume_value_label;

public:
  JackControlWidget (MorphPlanPtr plan, JackSynth *synth);

public slots:
  void on_plan_changed();
  void on_volume_changed (int new_volume);
};

}
