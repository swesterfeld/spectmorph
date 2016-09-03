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

int
VstUI::save_state (char **buffer)
{
  vector<unsigned char> data;
  MemOut mo (&data);
  morph_plan->save (&mo);

  string s = HexString::encode (data);

  *buffer = strdup (s.c_str()); // FIXME: leak
  return s.size() + 1; // save trailing 0 byte
}

void
VstUI::load_state (char *buffer)
{
  morph_plan->set_plan_str (buffer);
}
