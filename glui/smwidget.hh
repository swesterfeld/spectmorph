// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WIDGET_HH
#define SPECTMORPH_WIDGET_HH

#include <vector>
#include <cairo.h>
#include <stdio.h>

#include "smsignal.hh"

namespace SpectMorph
{

enum class TextAlign {
  LEFT,
  CENTER,
  RIGHT
};

struct Window;
class ScrollView;

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
  Rect
  intersection (const Rect& r)
  {
    // lower left corner
    double x1 = std::max (m_x, r.m_x);
    double y1 = std::max (m_y, r.m_y);

    // upper right corner
    double x2 = std::min (m_x + m_width,  r.m_x + r.m_width);
    double y2 = std::min (m_y + m_height, r.m_y + r.m_height);

    // FIXME: maybe special case the "no intersection at all" rectangle
    return Rect (x1, y1, std::max (x2 - x1, 0.0), std::max (y2 - y1, 0.0));
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
  OPERATOR_BG
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
};

struct Widget : public SignalReceiver
{
private:
  bool m_enabled = true;
  bool m_visible = true;
  Color m_background_color;

protected:
  void remove_child (Widget *child);

public:
  Widget *parent;
  double x, y, width, height;

  std::vector<Widget *> children;

  void debug_fill (cairo_t *cr)
  {
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_set_source_rgb (cr, 0.6, 0.6, 0.6);
    cairo_fill (cr);
  }

  Widget (Widget *parent, double x, double y, double width, double height);
  Widget (Widget *parent) :
    Widget (parent, 0, 0, 300, 100)
  {
  }
  virtual ~Widget();

  virtual void
  draw (cairo_t *cr);
  virtual bool
  clipping()
  {
    // clipping for draw() - enabled by default
    return true;
  }

  virtual void motion (double x, double y)
  {
  }
  virtual void
  mouse_press (double x, double y)
  {
  }
  virtual void
  mouse_release (double x, double y)
  {
  }
  virtual void
  scroll (double dx, double dy)
  {
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
  update()
  {
    if (parent)
      parent->update();
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
    m_enabled = e;
  }
  bool
  enabled() const
  {
    return m_enabled;
  }
  void
  set_visible (bool v)
  {
    m_visible = v;
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
  double abs_x() const;
  double abs_y() const;

  Rect   abs_visible_rect();
};

}

#endif
