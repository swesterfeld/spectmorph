// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WIDGET_HH
#define SPECTMORPH_WIDGET_HH

#include <vector>
#include <cairo.h>
#include <stdio.h>
#include <math.h>

#include "pugl/pugl.h"
#include "smsignal.hh"

namespace SpectMorph
{

enum class TextAlign {
  LEFT,
  CENTER,
  RIGHT
};

enum MouseButton  {
  NO_BUTTON = 0,
  LEFT_BUTTON = 1,
  MIDDLE_BUTTON = 2,
  RIGHT_BUTTON = 4
};

class Window;
class ScrollView;
class Timer;

class Rect
{
  double m_x;
  double m_y;
  double m_width;
  double m_height;
public:
  Rect() :
    m_x (0), m_y (0), m_width (0), m_height (0)
  {
  }
  Rect (double x, double y, double width, double height) :
    m_x (x), m_y (y), m_width (width), m_height (height)
  {
  }
  void
  move_to (double x, double y)
  {
    m_x = x;
    m_y = y;
  }
  bool
  contains (double x, double y)
  {
    return (x >= m_x) && (y >= m_y) && (x < m_x + m_width) && (y < m_y + m_height);
  }
  double
  x() const
  {
    return m_x;
  }
  double
  y() const
  {
    return m_y;
  }
  double
  width() const
  {
    return m_width;
  }
  double
  height() const
  {
    return m_height;
  }
  bool
  empty() const
  {
    return m_width * m_height == 0;
  }
  Rect
  intersection (const Rect& r)
  {
    // upper left corner
    double x1 = std::max (m_x, r.m_x);
    double y1 = std::max (m_y, r.m_y);

    // lower right corner
    double x2 = std::min (m_x + m_width,  r.m_x + r.m_width);
    double y2 = std::min (m_y + m_height, r.m_y + r.m_height);

    // FIXME: maybe special case the "no intersection at all" rectangle
    return Rect (x1, y1, std::max (x2 - x1, 0.0), std::max (y2 - y1, 0.0));
  }
  Rect
  rect_union (const Rect &r)
  {
    if (r.empty())
      return *this;
    if (empty())
      return r;

    // upper left corner
    double x1 = std::min (m_x, r.m_x);
    double y1 = std::min (m_y, r.m_y);

    // lower right corner
    double x2 = std::max (m_x + m_width,  r.m_x + r.m_width);
    double y2 = std::max (m_y + m_height, r.m_y + r.m_height);

    return Rect (x1, y1, x2 - x1, y2 - y1);
  }
};

class Point
{
  double m_x;
  double m_y;

public:
  Point() :
    m_x (0), m_y (0)
  {
  }
  Point (double x, double y) :
    m_x (x), m_y (y)
  {
  }
  double
  x() const
  {
    return m_x;
  }
  double
  y() const
  {
    return m_y;
  }
  double
  distance (Point p) const
  {
    const double dx = m_x - p.x();
    const double dy = m_y - p.y();

    return sqrt (dx * dx + dy * dy);
  }
};

enum class ThemeColor
{
  FRAME,
  MENU_BG,
  MENU_ITEM,
  CHECKBOX,
  SLIDER,
  WINDOW_BG,
  OPERATOR_BG,
  TEXT
};

class Color
{
  bool   m_valid = false;
  double m_red = 0;
  double m_green = 0;
  double m_blue = 0;

public:
  Color()
  {
  }
  Color (ThemeColor theme_color)
  {
    switch (theme_color)
      {
        case ThemeColor::FRAME:     set_rgb (0.8, 0.8, 0.8);  break;
        case ThemeColor::MENU_BG:   set_rgb (0.3, 0.3, 0.3);  break;
        case ThemeColor::MENU_ITEM: set_rgb (1, 0.6, 0.0);    break;
        case ThemeColor::CHECKBOX:  set_rgb (0.1, 0.7, 0.1);  break;
        case ThemeColor::SLIDER:    set_rgb (0.1, 0.7, 0.1);  break;
        case ThemeColor::WINDOW_BG: set_rgb (0.2, 0.2, 0.2);  break;
        case ThemeColor::OPERATOR_BG: set_rgb (0.2, 0.2, 0.2); break;
        case ThemeColor::TEXT:      set_rgb (1.0, 1.0, 1.0);  break;
      }
  }
  Color (double r, double g, double b)
  {
    set_rgb (r, g, b);
  }
  operator bool()
  {
    return m_valid;
  }
  static Color null()
  {
    return Color(); // not valid
  }
  double red() const
  {
    return m_red;
  }
  double green() const
  {
    return m_green;
  }
  double blue() const
  {
    return m_blue;
  }
  void
  set_rgb (double r, double g, double b)
  {
    m_red = r;
    m_green = g;
    m_blue = b;
    m_valid = true;
  }
  void set_hsv (double h, double s, double v);
  void get_hsv (double *h, double *s, double *v);
  Color lighter (double factor = 130);
  Color darker (double factor = 130);

  /* comparision */
  bool
  operator== (const Color& other) const
  {
    return other.m_valid == m_valid && other.m_red == m_red && other.m_green == m_green && other.m_blue == m_blue;
  }
  bool
  operator!= (const Color& other) const
  {
    return !(other == *this);
  }
};

struct Widget : public SignalReceiver
{
private:
  bool m_enabled = true;
  bool m_visible = true;
  Color m_background_color;
  std::vector<Timer *> timers;

protected:
  void remove_child (Widget *child);

public:
  Widget *parent;

  double m_x;
  double m_y;
  double m_width;
  double m_height;

  std::vector<Widget *> children;

  void debug_fill (cairo_t *cr)
  {
    cairo_rectangle (cr, 0, 0, width(), height());
    cairo_set_source_rgb (cr, 0.6, 0.6, 0.6);
    cairo_fill (cr);
  }

  Widget (Widget *parent, double x, double y, double width, double height);
  Widget (Widget *parent) :
    Widget (parent, 0, 0, 300, 100)
  {
  }
  virtual ~Widget();

  struct DrawEvent
  {
    cairo_t *cr;

    Rect     rect; // only valid for clipping() == true
  };
  struct MouseEvent
  {
    double        x = 0;
    double        y = 0;
    MouseButton   button = NO_BUTTON;
    unsigned      buttons = 0;
    bool          double_click = false;
  };

  virtual void draw (const DrawEvent& draw);

  virtual bool
  clipping()
  {
    // clipping for draw() - enabled by default
    return true;
  }

  virtual void
  mouse_move (const MouseEvent& event)
  {
  }
  virtual void
  mouse_press (const MouseEvent& event)
  {
  }
  virtual void
  mouse_release (const MouseEvent& event)
  {
  }
  virtual bool
  scroll (double dx, double dy)
  {
    return false;
  }
  virtual void
  enter_event()
  {
  }
  virtual void
  leave_event()
  {
  }
  virtual void
  focus_event()
  {
  }
  virtual void
  focus_out_event()
  {
  }
  virtual void
  key_press_event (const PuglEventKey& key_event)
  {
  }
  virtual Window *
  window()
  {
    return parent ? parent->window() : nullptr;
  }
  virtual ScrollView *
  scroll_view()
  {
    return parent ? parent->scroll_view() : nullptr;
  }
  void
  set_enabled (bool e)
  {
    if (e == m_enabled)
      return;

    m_enabled = e;
    update_with_children();
  }
  bool
  enabled() const
  {
    return m_enabled;
  }
  void
  set_visible (bool v)
  {
    if (v == m_visible)
      return;

    m_visible = v;
    update_with_children();
  }
  bool
  visible() const
  {
    return m_visible;
  }
  void
  set_background_color (Color color)
  {
    m_background_color = color;
  }
  Color
  background_color() const
  {
    return m_background_color;
  }
  bool
  recursive_enabled() const
  {
    if (!m_enabled)
      return false;
    if (parent)
      return parent->recursive_enabled();
    return true;
  }
  double
  x() const
  {
    return m_x;
  }
  double
  y() const
  {
    return m_y;
  }
  double
  width() const
  {
    return m_width;
  }
  double
  height() const
  {
    return m_height;
  }
  double abs_x() const;
  double abs_y() const;
  void set_x (double x);
  void set_y (double y);
  void set_width (double width);
  void set_height (double height);

  Signal<> signal_x_changed;
  Signal<> signal_y_changed;
  Signal<> signal_width_changed;
  Signal<> signal_height_changed;

  Rect   abs_visible_rect();
  void   update (double x, double y, double width, double height);
  void   update();
  void   update_with_children();
  void   update_full();
  void   delete_later();
  void   add_timer (Timer *timer);
  void   remove_timer (Timer *timer);
};

}

#endif
