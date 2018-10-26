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
  double                        jack_mix_freq;
  jack_port_t                  *input_port;
  std::vector<jack_port_t *>    output_ports;
  std::vector<jack_port_t *>    control_ports;

  double                        m_volume;

  MidiSynth                    *midi_synth;

  std::mutex                    m_new_plan_mutex;
  MorphPlanPtr                  m_new_plan;
  double                        m_new_volume;
  bool                          m_voices_active;

  bool                          m_inst_edit_changed = false;
  bool                          m_inst_edit_active = false;
  std::string                   m_inst_edit_filename;
  bool                          m_inst_edit_original_samples = false;

public:
  JackSynth (jack_client_t *client);
  ~JackSynth();

  void preinit_plan (MorphPlanPtr plan);
  void change_plan (MorphPlanPtr plan);
  void change_volume (double new_volume);
  void handle_inst_edit_update (bool active, const std::string& filename, bool original_samples);
  bool voices_active();
  int  process (jack_nframes_t nframes);
};

class JackControl : public SignalReceiver
{
  MorphPlanControl  *m_control_widget;
  JackSynth         *synth;
  MorphPlanPtr       morph_plan;

public:
  JackControl (MorphPlanPtr plan, MorphPlanWindow& window, MorphPlanControl *control_widget, JackSynth *synth);

  void update_led();

/* slots: */
  void on_plan_changed();
  void on_volume_changed (double d);
  void on_handle_inst_edit_update (bool active, const std::string& filename, bool original_samples);
};

}

#endif
