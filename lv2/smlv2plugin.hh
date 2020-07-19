// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_LV2_PLUGIN_HH
#define SPECTMORPH_LV2_PLUGIN_HH

#include "smsynthinterface.hh"

#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"

#define LV2_DEBUG(...) Debug::debug ("lv2", __VA_ARGS__)

namespace SpectMorph
{

class LV2Plugin : public LV2Common
{
public:
  // Port buffers
  const LV2_Atom_Sequence* midi_in;
  const float* control_1;
  const float* control_2;
  const float* control_3;
  const float* control_4;
  float*       left_out;
  float*       right_out;
  LV2_Atom_Sequence* notify_port;

  // Forge
  LV2_Atom_Forge        forge;

  // Logger
  LV2_Log_Log*          log;
  LV2_Log_Logger        logger;

  LV2Plugin (double mix_freq);

  void write_state_changed();

  struct TimePos {
    bool   have_speed    = false; // speed can be negative
    double speed         = 0;
    double bpm           = -1;
    double bar           = -1;
    double beats_per_bar = -1;
    double bar_beat      = -1;
    double beat_unit     = -1;
  };
  TimePos time_pos_from_object (const LV2_Atom_Object *obj);

  // SpectMorph stuff
  Project         project;
};

}

#endif /* SPECTMORPH_LV2_PLUGIN_HH */
