// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smvstui.hh"
#include "smvstplugin.hh"
#include "smmemout.hh"
#include "smhexstring.hh"

#include <QWindow>
#include <QPushButton>
#include <QApplication>
#include <QTimer>

using namespace SpectMorph;

using std::string;
using std::vector;

VstUI::VstUI (const string& filename, VstPlugin *plugin) :
  morph_plan (new MorphPlan()),
  plugin (plugin)
{
  GenericIn *in = StdioIn::open (filename);
  if (!in)
    {
      g_printerr ("Error opening '%s'.\n", filename.c_str());
      exit (1);
    }
  morph_plan->load (in);
  delete in;

  connect (morph_plan.c_ptr(), SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));
}

bool
VstUI::open (WId win_id)
{
  widget = new MorphPlanWindow (morph_plan, "!title!");
  widget->winId();
  widget->windowHandle()->setParent (QWindow::fromWinId (win_id));
  widget->show();
  rectangle.top = 0;
  rectangle.left = 0;
  rectangle.bottom = widget->height();
  rectangle.right = widget->width();

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
}

void
VstUI::idle()
{
  QApplication::processEvents();
}

void
VstUI::on_plan_changed()
{
  plugin->change_plan (morph_plan->clone());

  QTimer::singleShot (20, this, SLOT (on_update_window_size()));
}

void
VstUI::on_update_window_size()
{
  const int width = widget->minimumWidth();
  const int height = widget->minimumHeight();

  if (height != rectangle.bottom || height != rectangle.right)
    {
      rectangle.bottom = height;
      rectangle.right  = width;

      widget->resize (width, height);
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
    out_file.write_float ("volume",    plugin->get_parameter_value (VstPlugin::PARAM_VOLUME));
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
          plugin->set_parameter_value (VstPlugin::PARAM_VOLUME, in_file.event_float());
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
}
