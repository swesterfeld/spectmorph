// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smwidget.hh"
#include "smcurve.hh"
#include "smlabel.hh"
#include "smcheckbox.hh"
#include "smcombobox.hh"
#include "smvoicestatus.hh"

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
public:
  enum class Type {
    ENVELOPE,
    LFO,
    KEY_TRACK
  };
private:
  Curve m_curve;
  Type type = Type::KEY_TRACK;
  double start_x = 0;
  double start_y = 0;
  double end_x = 0;
  double end_y = 0;
  int highlight_index = -1;
  int highlight_seg_index = -1;
  int drag_index = -1;
  int drag_marker = -1;
  double drag_slope_y = 0;
  double drag_slope_slope = 0;
  double drag_slope_factor = 0;
  bool highlight = false;
  enum DragType {
    DRAG_NONE,
    DRAG_POINT,
    DRAG_SLOPE,
    DRAG_MARKER_START,
    DRAG_MARKER_END,
    DRAG_MARKER_BOTH
  } drag_type = DRAG_NONE;
  DragType highlight_type = DRAG_NONE;

  MorphOperator *control_op = nullptr;

  Point curve_point_to_xy (const Curve::Point& p);
  int find_closest_curve_index (const Point& p);
  int find_closest_segment_index (const Point& p);
  CurveGridLabel *x_grid_label = nullptr;
  Label          *cross_label = nullptr;
  CurveGridLabel *y_grid_label = nullptr;
  CheckBox       *snap_checkbox = nullptr;
  ComboBox       *loop_combobox = nullptr;
  Label          *note_label = nullptr;

  double grid_snap (double p, double start_p, double end_p, int n);
  std::string loop_to_text (Curve::Loop loop);
  Curve::Loop text_to_loop (const std::string& text);

  void update_highlight_type (const Point& p);

  void on_loop_changed();
  void on_update_geometry();

public:
  MorphCurveWidget (Widget *parent, MorphOperator *control_op, const Curve& initial_curve, Type type);

  void draw (const DrawEvent& devent) override;

  void mouse_press (const MouseEvent& event) override;
  void mouse_release (const MouseEvent& event) override;
  void mouse_move (const MouseEvent& event) override;

  void enter_event() override;
  void leave_event() override;

  Curve curve() const;

  void on_voice_status_changed (VoiceStatus *voice_status);

  Signal<> signal_curve_changed;
};

}

