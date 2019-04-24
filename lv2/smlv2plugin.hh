// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_LV2_PLUGIN_HH
#define SPECTMORPH_LV2_PLUGIN_HH

#include "smsynthinterface.hh"

#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
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
  float*       left_out;
  float*       right_out;
  LV2_Atom_Sequence* notify_port;

  // Logger
  LV2_Log_Log*          log;
  LV2_Log_Logger        logger;

  LV2_Worker_Schedule*  schedule;

  LV2Plugin (double mix_freq);

  // SpectMorph stuff
  Project         project;
  double          mix_freq;
  double          volume;

  void set_volume (double new_volume);

  Signal<> signal_post_load;
};

}

#endif /* SPECTMORPH_LV2_PLUGIN_HH */
