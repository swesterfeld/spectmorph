// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smwidget.hh"
#include "smcurve.hh"
#include "smlabel.hh"
#include "smcheckbox.hh"

namespace SpectMorph
{

class CurveGridLabel : public Label
{
  int m_n = 1;
  int drag_start_n = -1;
  double drag_start_y;
public:
  CurveGridLabel (Widget *widget, int n) :
    Label (widget, string_printf ("%d", n))
  {
    m_n = n;
  }
  int n() const { return m_n; }
  void
  mouse_press (const MouseEvent& event)
  {
    if (event.button == LEFT_BUTTON)
      {
        drag_start_y = event.y;
        drag_start_n = m_n;
      }
  }
  void
  mouse_move (const MouseEvent& event)
  {
    if (drag_start_n >= 0)
      {
        int n = lrint (std::clamp ((drag_start_y - event.y) / 25 + drag_start_n, 1.0, 64.0));
        if (n != m_n)
          {
            m_n = n;
            set_text (string_printf ("%d", m_n));
            signal_value_changed();
          }
      }
  }
  void
  mouse_release (const MouseEvent& event)
  {
    if (event.button == LEFT_BUTTON)
      drag_start_n = -1;
  }
  Signal<> signal_value_changed;
};

class MorphCurveWidget : public Widget
{
  Curve m_curve;
  double start_x = 0;
  double start_y = 0;
  double end_x = 0;
  double end_y = 0;
  int highlight_index = -1;
  int old_highlight_index = -1;
  int highlight_seg_index = -1;
  int old_highlight_seg_index = -1;
  int drag_index = -1;
  double drag_slope_y = 0;
  double drag_slope_slope = 0;
  double drag_slope_factor = 0;
  bool highlight = false;
  Point last_event_pos;
  enum DragType {
    DRAG_NONE,
    DRAG_POINT,
    DRAG_SLOPE
  } drag_type = DRAG_NONE;

  Point curve_point_to_xy (const Curve::Point& p);
  int find_closest_curve_index (const Point& p);
  int find_closest_segment_index (const Point& p);
  CurveGridLabel *x_grid_label;
  Label          *cross_label;
  CurveGridLabel *y_grid_label;
  CheckBox       *snap_checkbox;

  double grid_snap (double p, double start_p, double end_p, int n);

public:
  MorphCurveWidget (Widget *parent, const Curve& initial_curve);

  void draw (const DrawEvent& devent) override;

  void mouse_press (const MouseEvent& event) override;
  void mouse_release (const MouseEvent& event) override;
  void mouse_move (const MouseEvent& event) override;

  void enter_event() override;
  void leave_event() override;

  Curve curve() const;

  Signal<> signal_curve_changed;
};

}

