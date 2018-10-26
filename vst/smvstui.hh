// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
  MorphPlanWindow      *widget;
  MorphPlanPtr          morph_plan;
  VstPlugin            *plugin;

public:
  VstUI (MorphPlanPtr plan, VstPlugin *plugin);

  bool open (PuglNativeWindow win_id);
  bool getRect (ERect** rect);
  void close();
  void idle();

  int   save_state (char **ptr);
  void  load_state (char *ptr);

/* slots: */
  void on_plan_changed();
  void on_update_window_size();
  void on_volume_changed (double new_volume);
  void on_inst_edit_update (bool active, const std::string& filename, bool orig_samples);
};

}

#endif
