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

/* Color conversion from Rapicorn */
// This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0
void
Color::get_hsv (double *huep,           /* 0..360: 0=red, 120=green, 240=blue */
                double *saturationp,    /* 0..1 */
                double *valuep)         /* 0..1 */
{
  double r = red(), g = green(), b = blue();
  double value = MAX (MAX (r, g), b);
  double delta = value - MIN (MIN (r, g), b);
  double saturation = value == 0 ? 0 : delta / value;
  double hue = 0;
  if (saturation && huep)
    {
      if (r == value)
        {
          hue = 0 + 60 * (g - b) / delta;
          if (hue <= 0)
            hue += 360;
        }
      else if (g == value)
        hue = 120 + 60 * (b - r) / delta;
      else /* b == value */
        hue = 240 + 60 * (r - g) / delta;
    }
  if (huep)
    *huep = hue;
  if (saturationp)
    *saturationp = saturation;
  if (valuep)
    *valuep = value;
}

void
Color::set_hsv (double hue,             /* 0..360: 0=red, 120=green, 240=blue */
                double saturation,      /* 0..1 */
                double value)           /* 0..1 */
{
  uint center = int (hue / 60);
  double frac = hue / 60 - center;
  double v1s = value * (1 - saturation);
  double vsf = value * (1 - saturation * frac);
  double v1f = value * (1 - saturation * (1 - frac));
  switch (center)
    {
    case 6:
    case 0: /* red */
      set_rgb (value, v1f, v1s);
      break;
    case 1: /* red + green */
      set_rgb (vsf, value, v1s);
      break;
    case 2: /* green */
      set_rgb (v1s, value, v1f);
      break;
    case 3: /* green + blue */
      set_rgb (v1s, vsf, value);
      break;
    case 4: /* blue */
      set_rgb (v1f, v1s, value);
      break;
    case 5: /* blue + red */
      set_rgb (value, v1s, vsf);
      break;
    }
}
