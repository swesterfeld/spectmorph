// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwidget.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

using std::string;

static LeakDebugger leak_debugger ("SpectMorph::Widget");

Widget::Widget (Widget *parent, double x, double y, double width, double height) :
  parent (parent), x (x), y (y), width (width), height (height)
{
  leak_debugger.add (this);

  if (parent)
    parent->children.push_back (this);
}

Widget::~Widget()
{
  leak_debugger.del (this);
}


