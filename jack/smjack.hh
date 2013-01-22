/*
 * Copyright (C) 2010-2013 Stefan Westerfeld
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

#include "smmorphplanwindow.hh"
#include "smmorphplansynth.hh"
#include "smmorphplanvoice.hh"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>

#include <jack/jack.h>

class Voice
{
public:
  enum State {
    STATE_IDLE,
    STATE_ON,
    STATE_RELEASE
  };
  SpectMorph::MorphPlanVoice *mp_voice;

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

  SpectMorph::MorphPlanSynth   *morph_plan_synth;
  std::vector<Voice>            voices;
  std::vector<Voice*>           active_voices;
  std::vector<Voice*>           release_voices;

  Birnet::Mutex                 m_new_plan_mutex;
  SpectMorph::MorphPlanPtr      m_new_plan;
  double                        m_new_volume;

public:
  JackSynth();
  ~JackSynth();

  void init (jack_client_t *client, SpectMorph::MorphPlanPtr morph_plan);
  void preinit_plan (SpectMorph::MorphPlanPtr plan);
  void change_plan (SpectMorph::MorphPlanPtr plan);
  void change_volume (double new_volume);
  int  process (jack_nframes_t nframes);
  void reschedule();
};

class JackWindow : public QWidget
{
  Q_OBJECT

  QLabel         *inst_label;
  QPushButton    *inst_button;

  QLabel         *volume_label;
  QSlider        *volume_slider;
  QLabel         *volume_value_label;

  SpectMorph::MorphPlanWindow inst_window;

  jack_client_t  *client;

  JackSynth       synth;
public:
  JackWindow (SpectMorph::MorphPlanPtr plan, const std::string& title);
  ~JackWindow();

  void closeEvent (QCloseEvent *event);

public slots:
  void on_volume_changed (int new_volume);
  void on_edit_clicked();
};


