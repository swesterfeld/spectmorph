// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_LV2_PLUGIN_HH
#define SPECTMORPH_LV2_PLUGIN_HH

#include "smsynthinterface.hh"

#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"

namespace SpectMorph
{

class LV2Plugin : public LV2Common,
                  public SynthInterface
{
public:
  // Port buffers
  const LV2_Atom_Sequence* midi_in;
  const float* control_1;
  const float* control_2;
  float*       left_out;
  float*       right_out;
  LV2_Atom_Sequence* notify_port;

  // Logger
  LV2_Log_Log*          log;
  LV2_Log_Logger        logger;

  LV2_Worker_Schedule*  schedule;

  LV2_Atom_Forge  forge;

  // Forge frame for notify port
  LV2_Atom_Forge_Frame notify_frame;

  LV2Plugin (double mix_freq);

  // SynthInterface
  void synth_take_control_event (SynthControlEvent *event) override;
  std::vector<std::string> notify_take_events() override;

  // SpectMorph stuff
  Project         project;
  double          mix_freq;
  double          volume;
  std::mutex      new_plan_mutex;
  MorphPlanPtr    new_plan;
  MidiSynth       midi_synth;
  std::string     plan_str;
  bool            voices_active;
  bool            send_settings_to_ui;

  ControlEventVector control_events;
  std::vector<std::string> out_events;

  void update_plan (const std::string& new_plan_str);
};

}

#endif /* SPECTMORPH_LV2_PLUGIN_HH */
