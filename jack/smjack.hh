// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_JACK_HH
#define SPECTMORPH_JACK_HH

#include "smmorphplanwindow.hh"
#include "smmorphplancontrol.hh"
#include "smmorphplansynth.hh"
#include "smmorphplanvoice.hh"
#include "smmidisynth.hh"
#include "smled.hh"

#include <jack/jack.h>

namespace SpectMorph
{

class JackSynth
{
protected:
  jack_port_t                  *input_port;
  std::vector<jack_port_t *>    output_ports;
  std::vector<jack_port_t *>    control_ports;

  double                        m_volume;

  Project                      *m_project;

  double                        m_new_volume;

public:
  JackSynth (jack_client_t *client, Project *project);

  void change_volume (double new_volume);
  int  process (jack_nframes_t nframes);
};

class JackControl : public SignalReceiver
{
  MorphPlanControl  *m_control_widget;
  JackSynth         *synth;

public:
  JackControl (MorphPlanWindow& window, MorphPlanControl *control_widget, JackSynth *synth);

/* slots: */
  void on_volume_changed (double d);
};

}

#endif
