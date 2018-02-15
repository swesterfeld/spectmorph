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

  if (sview && sview->is_scroll_child (this))
    {
      const double delta_y = sview->abs_y() - abs_y() + 2;
      const double delta_x = sview->abs_x() - abs_x() + 2;

      const double max_height = sview->view_height + delta_y - 4;
      const double max_width = sview->view_width + delta_x - 4;

      return Rect (max (delta_x, 0.0) + abs_x(), max (delta_y, 0.0) + abs_y(), min (width, max_width), min (height, max_height));
    }
  else
    {
      return Rect (abs_x(), abs_y(), width, height);
    }
}
