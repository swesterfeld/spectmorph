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
struct ScrollView;

class Rect
{
  double m_x;
  double m_y;
  double m_width;
  double m_height;
public:
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
};

struct Widget : public SignalReceiver
{
private:
  bool m_enabled = true;

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
  draw (cairo_t *cr)
  {
    cairo_set_source_rgb (cr, 0.2, 0.2, 0.2);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);
  }
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
  double abs_x() const;
  double abs_y() const;

  Rect   abs_visible_rect();
};

}

#endif
