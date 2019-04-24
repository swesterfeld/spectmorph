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
  LV2UI_Resize         *ui_resize;
  EventLoop             event_loop;

public:
  LV2UI (PuglNativeWindow parent_win_id, LV2UI_Resize *ui_resize, LV2Plugin *plugin);
  ~LV2UI();

  MorphPlanWindow      *window;

  void port_event (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void*  buffer);
  void idle();

/* slots: */
  void on_update_window_size();
};

}

#endif /* SPECTMORPH_LV2_UI_HH */

