// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_VST_UI_HH
#define SPECTMORPH_VST_UI_HH

#include "smvstcommon.hh"

#include "smmorphplanwindow.hh"
#include "smmorphplancontrol.hh"
#include "smmorphplan.hh"

namespace SpectMorph
{

struct VstPlugin;

class VstUI : public SignalReceiver
{
  ERect                 rectangle;
  MorphPlanWindow      *widget = nullptr;
  EventLoop            *event_loop = nullptr;
  MorphPlanPtr          morph_plan;
  VstPlugin            *plugin = nullptr;

public:
  VstUI (MorphPlanPtr plan, VstPlugin *plugin);

  bool open (PuglNativeWindow win_id);
  bool getRect (ERect** rect);
  void close();
  void idle();

/* slots: */
  void on_plan_changed();
  void on_update_window_size();
};

}

#endif
