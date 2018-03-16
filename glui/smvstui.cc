// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smvstui.hh"
#include "smvstplugin.hh"
#include "smmemout.hh"
#include "smhexstring.hh"
#include "smutils.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

VstUI::VstUI (MorphPlanPtr plan, VstPlugin *plugin) :
  widget (nullptr),
  control_widget (nullptr),
  morph_plan (plan),
  plugin (plugin)
{
  connect (morph_plan->signal_plan_changed, this, &VstUI::on_plan_changed);
}

bool
VstUI::open (PuglNativeWindow win_id)
{
#if 0 //XXX
  widget = new MorphPlanWindow (morph_plan, "!title!");
  connect (widget, SIGNAL (update_size()), this, SLOT (on_update_window_size()));

  control_widget = new MorphPlanControl (morph_plan);
  control_widget->set_volume (plugin->volume());
  connect (control_widget, SIGNAL (volume_changed (double)), this, SLOT (on_volume_changed (double)));

  widget->add_control_widget (control_widget);

  widget->winId();
  widget->windowHandle()->setParent (QWindow::fromWinId (win_id));
  widget->show();
#endif
  widget = new MorphPlanWindow (win_id, false, morph_plan);
  connect (widget->signal_update_size, this, &VstUI::on_update_window_size);

  control_widget = widget->add_control_widget();
  control_widget->set_volume (plugin->volume());
  connect (control_widget->signal_volume_changed, this, &VstUI::on_volume_changed);

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
  delete control_widget;
  control_widget = nullptr;

  delete widget;
  widget = nullptr;
}

void
VstUI::idle()
{
#if 0 //XXX
  if (control_widget)
    control_widget->set_led (plugin->voices_active());
#endif

  if (widget)
    widget->process_events();
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

      plugin->audioMaster (plugin->aeffect, audioMasterSizeWindow, width, height, 0, 0);
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

#if 0
  // XXX
  if (control_widget)
    control_widget->set_volume (plugin->volume());
#endif
}
