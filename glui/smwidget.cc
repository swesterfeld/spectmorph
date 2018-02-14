// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwidget.hh"
#include "smleakdebugger.hh"
#include "smwindow.hh"
#include "smscrollview.hh"
#include <glib.h>

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
  while (!children.empty())
    {
      delete children.front();
    }

  Window *win = window();
  if (win)
    win->on_widget_deleted (this);

  if (parent)
    parent->remove_child (this);
  leak_debugger.del (this);
}

void
Widget::remove_child (Widget *child)
{
  for (std::vector<Widget *>::iterator ci = children.begin(); ci != children.end(); ci++)
    if (*ci == child)
      {
        children.erase (ci);
        return;
      }
  g_assert_not_reached();
}

/* map relative to absolute coordinates */
double
Widget::abs_x() const
{
  if (!parent)
    return x;
  else
    {
      double scroll_x = 0;

      ScrollView *scroll_view = parent->is_scroll_view();
      if (scroll_view)
        scroll_x = scroll_view->scroll_x;

      return parent->abs_x() + x - scroll_x;
    }
}

double
Widget::abs_y() const
{
  if (!parent)
    return y;
  else
    {
      double scroll_y = 0;

      ScrollView *scroll_view = parent->is_scroll_view();
      if (scroll_view)
        scroll_y = scroll_view->scroll_y;

      return parent->abs_y() + y - scroll_y;
    }
}
