// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphcurvewidget.hh"
#include "smdrawutils.hh"

using namespace SpectMorph;

MorphCurveWidget::MorphCurveWidget (Widget *parent, const Curve& initial_curve) :
  Widget (parent),
  m_curve (initial_curve)
{
}


Point
MorphCurveWidget::curve_point_to_xy (const Curve::Point& p)
{
  return Point (start_x + p.x * (end_x - start_x), start_y + p.y * (end_y - start_y));
}

void
MorphCurveWidget::draw (const DrawEvent& devent)
{
  DrawUtils du (devent.cr);
  cairo_t *cr = devent.cr;

  du.round_box (0, 0, width(), height(), 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

  start_x = 10;
  end_x = width() - 10;
  start_y = height() - 10;
  end_y = 10;

  Color line_color (ThemeColor::SLIDER);
  line_color = line_color.lighter();

  du.set_color (line_color);
  cairo_set_line_width (cr, 1);
  for (double x = start_x; x < end_x + 1; x += 1)
    {
      double y = start_y + m_curve ((x - start_x) / (end_x - start_x)) * (end_y - start_y);
      if (x == start_x)
        cairo_move_to (cr, x, y);
      else
        cairo_line_to (cr, x, y);
    }
  cairo_stroke (cr);

  Point seg_handle (-1, -1);
  if (highlight_seg_index >= 0 && highlight_seg_index < int (m_curve.points.size()) - 1 && highlight)
    {
      auto p1 = m_curve.points[highlight_seg_index];
      auto p2 = m_curve.points[highlight_seg_index + 1];
      auto p = Curve::Point ({0.5 * (p1.x + p2.x), m_curve (0.5 * (p1.x + p2.x))});
      seg_handle = curve_point_to_xy (p);
      du.circle (seg_handle.x(), seg_handle.y(), 5, line_color);
    }
  auto circle_color = Color (1, 1, 1);
  for (auto c : m_curve.points)
    {
      auto p = curve_point_to_xy (c);
      du.circle (p.x(), p.y(), 5, circle_color);
    }

  Point point_handle = curve_point_to_xy (m_curve.points[highlight_index]);
  if (highlight_index >= 0 && highlight_index < int (m_curve.points.size()) && highlight)
    {
      point_handle = curve_point_to_xy (m_curve.points[highlight_index]);
    }
  if (highlight)
    {
      if (last_event_pos.distance (point_handle) < last_event_pos.distance (seg_handle) && last_event_pos.distance (point_handle) < 10)
        du.circle (point_handle.x(), point_handle.y(), 7, circle_color);
      if (last_event_pos.distance (seg_handle) < last_event_pos.distance (point_handle) && last_event_pos.distance (seg_handle) < 10)
        du.circle (seg_handle.x(), seg_handle.y(), 7, line_color);
    }
}

void
MorphCurveWidget::mouse_press (const MouseEvent& event)
{
  last_event_pos = Point (event.x, event.y);
  update();

  if (event.button == LEFT_BUTTON)
    {
      int index = find_closest_curve_index (Point { event.x, event.y });
      assert (index >= 0);
      Point point_handle = curve_point_to_xy (m_curve.points[index]);

      if (event.double_click)
        {
          if (point_handle.distance (Point (event.x, event.y)) < 10) // delete old point
            {
              // keep first and last point
              if (index > 0 && index < int (m_curve.points.size()) - 1)
                m_curve.points.erase (m_curve.points.begin() + index);
            }
          else
            {
              double px = std::clamp ((event.x - start_x) / (end_x - start_x), 0.0, 1.0);
              double py = std::clamp ((event.y - start_y) / (end_y - start_y), 0.0, 1.0);

              m_curve.points.emplace_back (Curve::Point ({ px, py }));
              std::stable_sort (m_curve.points.begin(), m_curve.points.end(), [] (auto pa, auto pb) { return pa.x < pb.x; });
            }
          signal_curve_changed();
        }
      else
        {
          Point seg_handle;
          if (highlight_seg_index >= 0 && highlight_seg_index < int (m_curve.points.size()) - 1)
            {
              auto p1 = m_curve.points[highlight_seg_index];
              auto p2 = m_curve.points[highlight_seg_index + 1];
              auto p = Curve::Point ({0.5 * (p1.x + p2.x), m_curve (0.5 * (p1.x + p2.x))});
              seg_handle = curve_point_to_xy (p);
              drag_slope_slope = m_curve.points[highlight_seg_index].slope;
              if (p1.y > p2.y)
                drag_slope_factor = -1;
              else
                drag_slope_factor = 1;
            }

          if (index >= 0)
            {
              drag_type = DRAG_NONE;
              if (last_event_pos.distance (point_handle) < last_event_pos.distance (seg_handle) && last_event_pos.distance (point_handle) < 10)
                drag_type = DRAG_POINT;
              if (last_event_pos.distance (seg_handle) < last_event_pos.distance (point_handle) && last_event_pos.distance (seg_handle) < 10)
                {
                  drag_type = DRAG_SLOPE;
                  drag_slope_y = event.y;
                }
            }

          drag_index = index;
        }
      update();
    }
}

void
MorphCurveWidget::mouse_move (const MouseEvent& event)
{
  last_event_pos = Point (event.x, event.y);
  update();

  Point event_p (event.x, event.y);

  if (drag_index >= 0 && drag_type == DRAG_POINT)
    {
      double px = std::clamp ((event.x - start_x) / (end_x - start_x), 0.0, 1.0);
      double py = std::clamp ((event.y - start_y) / (end_y - start_y), 0.0, 1.0);
      if (drag_index == 0)
        px = 0;
      if (drag_index > 0)
        px = std::max (m_curve.points[drag_index - 1].x, px);
      if (drag_index < int (m_curve.points.size()) - 1)
        px = std::min (m_curve.points[drag_index + 1].x, px);
      if (drag_index == int (m_curve.points.size()) - 1)
        px = 1;
      m_curve.points[drag_index].x = px;
      m_curve.points[drag_index].y = py;
      signal_curve_changed();
      update();
      return;
    }
  else if (drag_type == DRAG_SLOPE)
    {
      double diff = (event.y - drag_slope_y) / 50;
      double slope = drag_slope_slope + diff * drag_slope_factor;
      slope = std::clamp (slope, -1.0, 1.0);
      if (highlight_seg_index >= 0 && highlight_seg_index < int (m_curve.points.size()) - 1)
        m_curve.points[highlight_seg_index].slope = slope;
      signal_curve_changed();
      update();
      return;
    }

  highlight_index = find_closest_curve_index (event_p);
  highlight_seg_index = find_closest_segment_index (event_p);

  if (old_highlight_index != highlight_index || old_highlight_seg_index != highlight_seg_index)
    {
      update();
      old_highlight_index = highlight_index;
      old_highlight_seg_index = highlight_seg_index;
    }
}

int
MorphCurveWidget::find_closest_curve_index (const Point& p)
{
  double best_dist = 1e30;
  int index = -1;
  for (size_t i = 0; i < m_curve.points.size(); i++)
    {
      Point curve_p = curve_point_to_xy (m_curve.points[i]);
      double dist = curve_p.distance (p);
      if (dist < best_dist)
        {
          index = i;
          best_dist = dist;
        }
    }
  return index;
}

int
MorphCurveWidget::find_closest_segment_index (const Point& p)
{
  int index = -1;
  for (size_t i = 0; i < m_curve.points.size(); i++)
    {
      Point curve_p = curve_point_to_xy (m_curve.points[i]);
      if (curve_p.x() < p.x())
        index = i;
    }
  return index;
}

void
MorphCurveWidget::mouse_release (const MouseEvent& event)
{
  last_event_pos = Point (event.x, event.y);
  update();
  if (event.button == LEFT_BUTTON)
    {
      drag_index = -1;
      drag_type = DRAG_NONE;
    }
}

void
MorphCurveWidget::enter_event()
{
  highlight = true;
  update();
}

void
MorphCurveWidget::leave_event()
{
  highlight = false;
  update();
}

Curve
MorphCurveWidget::curve() const
{
  return m_curve;
}
