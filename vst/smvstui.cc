// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smvstui.hh"

#include <QWindow>
#include <QPushButton>
#include <QApplication>

using namespace SpectMorph;

using std::string;

VstUI::VstUI (const string& filename) :
    morph_plan (new MorphPlan())
{
  GenericIn *in = StdioIn::open (filename);
  if (!in)
    {
      g_printerr ("Error opening '%s'.\n", filename.c_str());
      exit (1);
    }
  morph_plan->load (in);
  delete in;
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
