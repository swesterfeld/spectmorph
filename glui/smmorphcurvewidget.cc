// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphcurvewidget.hh"
#include "smdrawutils.hh"
#include "smfixedgrid.hh"

using namespace SpectMorph;

using std::string;
using std::min;
using std::max;

MorphCurveWidget::MorphCurveWidget (Widget *parent, MorphOperator *control_op, const Curve& initial_curve, bool can_loop) :
  Widget (parent),
  m_curve (initial_curve),
  can_loop (can_loop),
  control_op (control_op)
{
  x_grid_label = new CurveGridLabel (parent, initial_curve.grid_x);
  y_grid_label = new CurveGridLabel (parent, initial_curve.grid_y);
  cross_label = new Label (parent, "x");
  cross_label->set_align (TextAlign::CENTER);
  x_grid_label->set_bold (true);
  x_grid_label->set_align (TextAlign::RIGHT);
  y_grid_label->set_bold (true);
  snap_checkbox = new CheckBox (parent, "Snap");
  snap_checkbox->set_checked (initial_curve.snap);
  if (can_loop)
    {
      loop_combobox = new ComboBox (this);
      connect (loop_combobox->signal_item_changed, this, &MorphCurveWidget::on_loop_changed);
      loop_combobox->add_item (loop_to_text (Curve::Loop::NONE));
      loop_combobox->set_text (loop_to_text (m_curve.loop));
      loop_combobox->add_item (loop_to_text (Curve::Loop::SUSTAIN));
      loop_combobox->add_item (loop_to_text (Curve::Loop::FORWARD));
      loop_combobox->add_item (loop_to_text (Curve::Loop::PING_PONG));
    }
  connect (x_grid_label->signal_value_changed,
    [this]() {
      m_curve.grid_x = x_grid_label->n();
      signal_curve_changed();
      update();
    });
  connect (y_grid_label->signal_value_changed,
    [this]() {
      m_curve.grid_y = y_grid_label->n();
      signal_curve_changed();
      update();
    });
  connect (snap_checkbox->signal_toggled,
    [this](bool checked) {
      m_curve.snap = checked;
      signal_curve_changed();
      update();
    });
  on_update_geometry();
  connect (signal_width_changed, this, &MorphCurveWidget::on_update_geometry);
  connect (signal_height_changed, this, &MorphCurveWidget::on_update_geometry);
}

void
MorphCurveWidget::on_update_geometry()
{
  FixedGrid grid;
  double yoffset = height() / 8;
  double xoffset = 0;
  grid.add_widget (snap_checkbox, xoffset, yoffset, 6, 2);
  xoffset += 6;
  grid.add_widget (x_grid_label, xoffset, yoffset, 2, 2);
  xoffset += 2;
  grid.add_widget (cross_label, xoffset, yoffset, 1, 2);
  xoffset += 1;
  grid.add_widget (y_grid_label, xoffset, yoffset, 2, 2);
  xoffset += 3;

  if (can_loop)
    grid.add_widget (loop_combobox, xoffset, yoffset - 1.5, 39 - xoffset, 3);
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

  double status_y_pixels = 16;
  du.round_box (0, 0, width(), height() - status_y_pixels, 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

  start_x = 10;
  end_x = width() - 10;
  start_y = height() - 10 - status_y_pixels;
  end_y = 10;

  cairo_set_line_width (cr, 1);
  if (m_curve.snap)
    {
      Color grid_color (0.4, 0.4, 0.4);
      du.set_color (grid_color);
      for (int i = 0; i <= x_grid_label->n(); i++)
        {
          double x = start_x + (end_x - start_x) * i / m_curve.grid_x;
          cairo_move_to (cr, x, start_y);
          cairo_line_to (cr, x, end_y);
          cairo_stroke (cr);
        }
      for (int i = 0; i <= y_grid_label->n(); i++)
        {
          double y = start_y + (end_y - start_y) * i / m_curve.grid_y;
          cairo_move_to (cr, start_x, y);
          cairo_line_to (cr, end_x, y);
          cairo_stroke (cr);
        }
    }
  /* draw loop markers */
  if (can_loop && m_curve.loop != Curve::Loop::NONE)
    {
      Color loop_marker_color = Color (0.7, 0.7, 1);
      if (highlight_type == DRAG_MARKER_START || highlight_type == DRAG_MARKER_BOTH)
        du.set_color (loop_marker_color.lighter());
      else
        du.set_color (loop_marker_color);

      double loop_start_x = start_x + (end_x - start_x) * m_curve.points[m_curve.loop_start].x;
      cairo_move_to (cr, loop_start_x, start_y);
      cairo_line_to (cr, loop_start_x, end_y);
      cairo_stroke (cr);
      if (m_curve.loop != Curve::Loop::SUSTAIN && m_curve.loop_start != m_curve.loop_end)
        {
          if (highlight_type == DRAG_MARKER_END)
            du.set_color (loop_marker_color.lighter());
          else
            du.set_color (loop_marker_color);

          double loop_end_x = start_x + (end_x - start_x) * m_curve.points[m_curve.loop_end].x;
          cairo_move_to (cr, loop_end_x, start_y);
          cairo_line_to (cr, loop_end_x, end_y);
          cairo_stroke (cr);
          /* lighten loop region */
          if (m_curve.loop == Curve::Loop::FORWARD || m_curve.loop == Curve::Loop::PING_PONG)
            {
              cairo_rectangle (cr, loop_start_x, start_y, loop_end_x - loop_start_x, end_y - start_y);
              cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.25);
              cairo_fill (cr);
            }
        }
    }

  Color line_color (ThemeColor::SLIDER);
  line_color = line_color.lighter();

  du.set_color (line_color);
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
      auto p = Curve::Point ({0.5f * (p1.x + p2.x), m_curve (0.5f * (p1.x + p2.x))});
      seg_handle = curve_point_to_xy (p);
      double radius = 5;
      if (highlight_type == DRAG_SLOPE)
        radius = 7;
      du.circle (seg_handle.x(), seg_handle.y(), radius, line_color);
    }

  auto circle_color = Color (1, 1, 1);
  for (int i = 0; i < int (m_curve.points.size()); i++)
    {
      auto p = curve_point_to_xy (m_curve.points[i]);
      double radius = 5;
      if (highlight_type == DRAG_POINT && highlight_index == i)
        radius = 7;
      du.circle (p.x(), p.y(), radius, circle_color);
    }
  for (auto status_point : voice_status_points)
    {
      auto p = curve_point_to_xy (status_point);
      du.circle (p.x(), p.y(), 7, circle_color);
    }
}

void
MorphCurveWidget::mouse_press (const MouseEvent& event)
{
  update_highlight_type (Point (event.x, event.y));

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
                {
                  m_curve.points.erase (m_curve.points.begin() + index);
                  if (index < m_curve.loop_start)
                    m_curve.loop_start--;
                  if (index < m_curve.loop_end)
                    m_curve.loop_end--;
                }
            }
          else
            {
              float px = std::clamp ((event.x - start_x) / (end_x - start_x), 0.0, 1.0);
              float py = std::clamp ((event.y - start_y) / (end_y - start_y), 0.0, 1.0);

              int insert_pos = -1;
              for (int i = 0; i < int (m_curve.points.size()) - 1; i++)
                {
                  if (px < m_curve.points[i + 1].x && px > m_curve.points[i].x)
                    {
                      insert_pos = i;
                      break;
                    }
                }
              if (insert_pos >= 0)
                {
                  m_curve.points.insert (m_curve.points.begin() + insert_pos + 1, Curve::Point ({ px, py }));
                  if (insert_pos < m_curve.loop_start)
                    m_curve.loop_start++;
                  if (insert_pos < m_curve.loop_end)
                    m_curve.loop_end++;
                }
            }
          signal_curve_changed();
        }
      else
        {
          if (index >= 0)
            {
              drag_type = DRAG_NONE;
              if (highlight_type == DRAG_POINT)
                {
                  drag_type = DRAG_POINT;
                  drag_index = index;
                }
              else if (highlight_type == DRAG_MARKER_START || highlight_type == DRAG_MARKER_END || highlight_type == DRAG_MARKER_BOTH)
                {
                  drag_type = highlight_type;
                }
              else if (highlight_type == DRAG_SLOPE)
                {
                  drag_type = DRAG_SLOPE;
                  drag_slope_y = event.y;
                  if (highlight_seg_index >= 0 && highlight_seg_index < int (m_curve.points.size()) - 1)
                    {
                      auto p1 = m_curve.points[highlight_seg_index];
                      auto p2 = m_curve.points[highlight_seg_index + 1];
                      drag_slope_slope = m_curve.points[highlight_seg_index].slope;
                      if (p1.y > p2.y)
                        drag_slope_factor = -1;
                      else
                        drag_slope_factor = 1;
                    }

                }
            }
        }
      update();
    }
}

double
MorphCurveWidget::grid_snap (double p, double start_p, double end_p, int n)
{
  if (!m_curve.snap)
    return p;

  double best_diff = 15; // maximum distance in pixels to snap
  double snap_p = p;
  for (int i = 0; i <= n; i++)
    {
      double grid_p = start_p + (end_p - start_p) * i / n;
      double diff = std::abs (grid_p - p);
      if (diff < best_diff)
        {
          snap_p = grid_p;
          best_diff = diff;
        }
    }
  return snap_p;
}

void
MorphCurveWidget::mouse_move (const MouseEvent& event)
{
  Point event_p (event.x, event.y);

  if (drag_index >= 0 && drag_type == DRAG_POINT)
    {
      const double snap_x = grid_snap (event.x, start_x, end_x, m_curve.grid_x);
      const double snap_y = grid_snap (event.y, start_y, end_y, m_curve.grid_y);

      float px = std::clamp ((snap_x - start_x) / (end_x - start_x), 0.0, 1.0);
      float py = std::clamp ((snap_y - start_y) / (end_y - start_y), 0.0, 1.0);
      if (drag_index == 0)
        px = 0;
      if (drag_index > 0)
        px = max (m_curve.points[drag_index - 1].x, px);
      if (drag_index < int (m_curve.points.size()) - 1)
        px = min (m_curve.points[drag_index + 1].x, px);
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
  else if (drag_type == DRAG_MARKER_START || drag_type == DRAG_MARKER_END || drag_type == DRAG_MARKER_BOTH)
    {
      double best_dist = 1e30;
      int idx = -1;
      for (size_t i = 0; i < m_curve.points.size(); i++)
        {
          Point curve_p = curve_point_to_xy (m_curve.points[i]);
          double dist = std::abs (event_p.x() - curve_p.x());
          if (dist < best_dist)
            {
              idx = i;
              best_dist = dist;
            }
        }
      if (idx >= 0)
        {
          if (drag_type == DRAG_MARKER_START)
            {
              if (m_curve.loop == Curve::Loop::SUSTAIN)
                {
                  m_curve.loop_start = m_curve.loop_end = idx;
                }
              else
                {
                  m_curve.loop_start = min (idx, m_curve.loop_end);
                }
            }
          else if (drag_type == DRAG_MARKER_END)
            {
              m_curve.loop_end = max (idx, m_curve.loop_start);
            }
          else /* DRAG_MARKER_BOTH */
            {
              m_curve.loop_start = min (idx, drag_marker);
              m_curve.loop_end = max (idx, drag_marker);
            }
          signal_curve_changed();
        }
      update();
      return;
    }
  update_highlight_type (Point (event.x, event.y));
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
  update_highlight_type (Point (event.x, event.y));
  if (event.button == LEFT_BUTTON)
    {
      drag_index = -1;
      drag_type = DRAG_NONE;
    }
}

void
MorphCurveWidget::update_highlight_type (const Point& p)
{
  highlight_seg_index = -1;
  highlight_type = DRAG_NONE;

  int index = find_closest_curve_index (p);
  if (index >= 0)
    {
      Point point_handle = curve_point_to_xy (m_curve.points[index]);
      if (p.distance (point_handle) < 10)
        {
          highlight_type = DRAG_POINT;
          highlight_index = index;
        }
      if (highlight_type == DRAG_NONE)
        {
          highlight_seg_index = find_closest_segment_index (p);
          if (highlight_seg_index >= 0)
            {
              auto p1 = m_curve.points[highlight_seg_index];
              auto p2 = m_curve.points[highlight_seg_index + 1];
              auto pavg = Curve::Point ({0.5f * (p1.x + p2.x), m_curve (0.5f * (p1.x + p2.x))});
              Point seg_handle = curve_point_to_xy (pavg);
              if (p.distance (seg_handle) < p.distance (point_handle) && p.distance (seg_handle) < 10)
                highlight_type = DRAG_SLOPE;
            }
          if (highlight_type == DRAG_NONE && m_curve.loop != Curve::Loop::NONE)
            {
              double x = start_x + (end_x - start_x) * m_curve.points[m_curve.loop_start].x;
              double start_dist = std::abs (x - p.x());
              if (start_dist < 10)
                {
                  highlight_type = DRAG_MARKER_START;
                  drag_marker = m_curve.loop_start;
                }
              if (m_curve.loop == Curve::Loop::FORWARD || m_curve.loop == Curve::Loop::PING_PONG)
                {
                  double x = start_x + (end_x - start_x) * m_curve.points[m_curve.loop_end].x;
                  double end_dist = std::abs (x - p.x());
                  if (end_dist < 10)
                    {
                      if (highlight_type == DRAG_MARKER_START && drag_marker == m_curve.loop_end)
                        {
                          /* special case: both loop markers are on the same point */
                          highlight_type = DRAG_MARKER_BOTH;
                        }
                      else if (end_dist < start_dist)
                        {
                          highlight_type = DRAG_MARKER_END;
                          drag_marker = m_curve.loop_end;
                        }
                    }
                }
            }
        }
    }
  update(); // can be done cheaper
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

void
MorphCurveWidget::on_loop_changed()
{
  m_curve.loop = text_to_loop (loop_combobox->text());
  signal_curve_changed();
  update();
}

string
MorphCurveWidget::loop_to_text (Curve::Loop loop)
{
  switch (loop)
    {
      case Curve::Loop::NONE:        return "No Loop";
      case Curve::Loop::SUSTAIN:     return "Sustain Loop";
      case Curve::Loop::FORWARD:     return "Forward Loop";
      case Curve::Loop::PING_PONG:   return "Ping Pong Loop";
    }
  return ""; /* not found */
}

Curve::Loop
MorphCurveWidget::text_to_loop (const string& text)
{
  for (int i = 0; ; i++)
    {
      Curve::Loop loop { i };
      string txt = loop_to_text (loop);

      if (txt == text)
        return loop;
      if (txt == "")
        return Curve::Loop::NONE;
    }
}

void
MorphCurveWidget::on_voice_status_changed (VoiceStatus *voice_status)
{
  voice_status_points.clear();
  for (uint i = 0; i < voice_status->n_voices(); i++)
    {
      auto notify_values = voice_status->get_notify_values (control_op, i);
      if (notify_values.size())
        {
          assert (notify_values.size() == 2);
          voice_status_points.push_back (Curve::Point { notify_values[1], (notify_values[0] + 1) * 0.5f });
        }
    }
  update();
}
