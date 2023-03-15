// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smvstui.hh"
#include "smvstplugin.hh"
#include "smmemout.hh"
#include "smhexstring.hh"
#include "smutils.hh"
#include "smvstresize.hh"
#include "smeventloop.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

VstUI::VstUI (MorphPlan *plan, VstPlugin *plugin) :
  morph_plan (plan),
  plugin (plugin)
{
  // some hosts ask for the window size before creating the window
  int width, height;
  MorphPlanWindow::static_scaled_size (&width, &height);

  rectangle.top = 0;
  rectangle.left = 0;
  rectangle.bottom = height;
  rectangle.right = width;

  const int sizeWindowOk = plugin->audioMaster (plugin->aeffect, audioMasterCanDo, 0, 0, (void *) "sizeWindow", 0);

  VST_DEBUG ("ui: sizeWindow supported: %d\n", sizeWindowOk);
}

bool
VstUI::open (PuglNativeWindow win_id)
{
  event_loop = new EventLoop();
  widget = new MorphPlanWindow (*event_loop, "SpectMorph VST", win_id, false, morph_plan);
  connect (widget->signal_update_size, this, &VstUI::on_update_window_size);

  widget->show();

  int width, height;

  widget->get_scaled_size (&width, &height);

  rectangle.top = 0;
  rectangle.left = 0;
  rectangle.bottom = height;
  rectangle.right = width;

  return true;
}

bool
VstUI::getRect (ERect** rect)
{
  *rect = &rectangle;

  return true;
}

void
VstUI::close()
{
  delete widget;
  widget = nullptr;
  delete event_loop;
  event_loop = nullptr;
}

void
VstUI::idle()
{
  if (widget)
    event_loop->process_events();
}

void
VstUI::on_update_window_size()
{
  if (!widget)  // if editor window is not visible, ignore
    return;

  int width, height;
  widget->get_scaled_size (&width, &height);

  if (height != rectangle.bottom || height != rectangle.right)
    {
      rectangle.bottom = height;
      rectangle.right  = width;

      int rc = plugin->audioMaster (plugin->aeffect, audioMasterSizeWindow, width, height, 0, 0);
      if (rc == 0)
        vst_manual_resize (widget, width, height);

      VST_DEBUG ("ui: audioMasterSizeWindow returned %d\n", rc);
    }
}
