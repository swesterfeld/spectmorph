// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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

VstUI::VstUI (MorphPlanPtr plan, VstPlugin *plugin) :
  morph_plan (plan),
  plugin (plugin)
{
  connect (morph_plan->signal_plan_changed, this, &VstUI::on_plan_changed);

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
  widget = new MorphPlanWindow (*event_loop, "SpectMorph VST", win_id, false, morph_plan, plugin);
  connect (widget->signal_update_size, this, &VstUI::on_update_window_size);

  widget->control_widget()->set_volume (plugin->volume());
  connect (widget->control_widget()->signal_volume_changed, this, &VstUI::on_volume_changed);

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
    {
      widget->control_widget()->set_led (plugin->voices_active());
      event_loop->process_events();
    }
}

void
VstUI::on_plan_changed()
{
  plugin->change_plan (morph_plan->clone());
}

void
VstUI::on_volume_changed (double new_volume)
{
  plugin->set_volume (new_volume);
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

class VstExtraParameters : public MorphPlan::ExtraParameters
{
  VstPlugin *plugin;
public:
  VstExtraParameters (VstPlugin *plugin) :
    plugin (plugin)
  {
  }

  string section() { return "vst_parameters"; }

  void
  save (OutFile& out_file)
  {
    out_file.write_float ("control_1", plugin->get_parameter_value (VstPlugin::PARAM_CONTROL_1));
    out_file.write_float ("control_2", plugin->get_parameter_value (VstPlugin::PARAM_CONTROL_2));
    out_file.write_float ("volume",    plugin->volume());
  }

  void
  handle_event (InFile& in_file)
  {
    if (in_file.event() == InFile::FLOAT)
      {
        if (in_file.event_name() == "control_1")
          plugin->set_parameter_value (VstPlugin::PARAM_CONTROL_1, in_file.event_float());

        if (in_file.event_name() == "control_2")
          plugin->set_parameter_value (VstPlugin::PARAM_CONTROL_2, in_file.event_float());

        if (in_file.event_name() == "volume")
          plugin->set_volume (in_file.event_float());
      }
  }
};

int
VstUI::save_state (char **buffer)
{
  VstExtraParameters params (plugin);

  vector<unsigned char> data;
  MemOut mo (&data);
  morph_plan->save (&mo, &params);

  string s = HexString::encode (data);

  *buffer = strdup (s.c_str()); // FIXME: leak
  return s.size() + 1; // save trailing 0 byte
}

void
VstUI::load_state (char *buffer)
{
  VstExtraParameters params (plugin);

  vector<unsigned char> data;
  if (!HexString::decode (buffer, data))
    return;

  GenericIn *in = MMapIn::open_mem (&data[0], &data[data.size()]);
  morph_plan->load (in, &params);
  delete in;

  if (widget)
    widget->control_widget()->set_volume (plugin->volume());
}
