// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LV2_UI_HH
#define SPECTMORPH_LV2_UI_HH

#include "smmorphplanwindow.hh"
#include "smmorphplancontrol.hh"
#include "smlv2common.hh"
#include "smeventloop.hh"
#include "smlv2plugin.hh"

namespace SpectMorph
{

class LV2UI : public SignalReceiver,
              public LV2Common
{
  LV2Plugin            *plugin;
  std::string           current_plan;
  LV2UI_Resize         *ui_resize;
  std::vector<std::string>  notify_events;
  EventLoop             event_loop;

public:
  LV2UI (PuglNativeWindow parent_win_id, LV2UI_Resize *ui_resize, LV2Plugin *plugin);
  ~LV2UI();

  MorphPlanWindow      *window;

  void port_event (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void*  buffer);
  void idle();

  std::vector<std::string> notify_take_events();

/* slots: */
  void on_volume_changed (double new_volume);
  void on_update_window_size();
  void on_post_load();
};

}

#endif /* SPECTMORPH_LV2_UI_HH */

