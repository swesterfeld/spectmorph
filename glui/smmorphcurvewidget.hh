// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smwidget.hh"
#include "smcurve.hh"

namespace SpectMorph
{

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

