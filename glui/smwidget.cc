// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwidget.hh"
#include "smleakdebugger.hh"
#include "smwindow.hh"
#include "smscrollview.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::max;
using std::min;

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
    return parent->abs_x() + x;
}

double
Widget::abs_y() const
{
  if (!parent)
    return y;
  else
    return parent->abs_y() + y;
}

Rect
Widget::abs_visible_rect()
{
  ScrollView *sview = scroll_view();

  Rect visible_rect (abs_x(), abs_y(), width, height);

  if (sview && sview->is_scroll_child (this))
    {
      return visible_rect.intersection (sview->child_rect());
    }
  else
    {
      return visible_rect;
    }
}
