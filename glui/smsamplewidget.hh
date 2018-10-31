// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SAMPLE_WIDGET_HH
#define SPECTMORPH_SAMPLE_WIDGET_HH

#include "smwidget.hh"
#include "sminstrument.hh"

#include <map>

namespace SpectMorph
{

class SampleWidget : public Widget
{
  void
  draw_grid (const DrawEvent& devent)
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    du.set_color (Color (0.33, 0.33, 0.33));
    cairo_set_line_width (cr, 1);

    const double pad = 8;
    for (double y = pad; y < height - 4; y += pad)
      {
        cairo_move_to (cr, 0, y);
        cairo_line_to (cr, width, y);
        cairo_stroke (cr);
      }
    for (double x = pad; x < width - 4; x += pad)
      {
        if (x >= devent.rect.x() && x <= devent.rect.x() + devent.rect.width())
          {
            cairo_move_to (cr, x, 0);
            cairo_line_to (cr, x, height);
            cairo_stroke (cr);
          }
      }
  }
  double        vzoom = 1;
  Sample       *m_sample = nullptr;
  MarkerType    selected_marker = MARKER_NONE;
  bool          mouse_down = false;

  std::map<MarkerType, Rect> marker_rect;
  std::vector<float>         m_play_pointers;
public:
  SampleWidget (Widget *parent)
    : Widget (parent)
  {
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

    draw_grid (devent);

    /* redraw border to overdraw line endings */
    du.round_box (0, 0, width, height, 1, 5, Color (0.4, 0.4, 0.4), Color::null());

    if (!m_sample)
      return;

    const double length_ms = m_sample->wav_data.samples().size() / m_sample->wav_data.mix_freq() * 1000;
    const double clip_start_x = m_sample->get_marker (MARKER_CLIP_START) / length_ms * width;
    const double clip_end_x = m_sample->get_marker (MARKER_CLIP_END) / length_ms * width;
    const double loop_start_x = m_sample->get_marker (MARKER_LOOP_START) / length_ms * width;
    const double loop_end_x = m_sample->get_marker (MARKER_LOOP_END) / length_ms * width;
    const std::vector<float>& samples = m_sample->wav_data.samples();

    //du.set_color (Color (0.4, 0.4, 1.0));
    du.set_color (Color (0.9, 0.1, 0.1));
    for (int pass = 0; pass < 2; pass++)
      {
        int last_x_pixel = -1;
        float max_s = 0;
        float min_s = 0;
        cairo_move_to (cr, 0, height / 2);
        for (size_t i = 0; i < samples.size(); i++)
          {
            double dx = double (i) * width / samples.size();

            if (dx >= devent.rect.x() && dx <= devent.rect.x() + devent.rect.width())
              {
                int x_pixel = dx;
                max_s = std::max (samples[i], max_s);
                min_s = std::min (samples[i], min_s);
                if (x_pixel != last_x_pixel)
                  {
                    if (pass == 0)
                      cairo_line_to (cr, last_x_pixel, height / 2 + min_s * height / 2 * vzoom);
                    else
                      cairo_line_to (cr, last_x_pixel, height / 2 + max_s * height / 2 * vzoom);

                    last_x_pixel = x_pixel;
                    max_s = 0;
                    min_s = 0;
                  }
              }
          }
        cairo_line_to (cr, last_x_pixel, height / 2);
        cairo_close_path (cr);
        cairo_set_line_width (cr, 1);
        cairo_stroke_preserve (cr);
        cairo_fill (cr);
      }

    /* lighten loop region */
    if (m_sample->loop() == Sample::Loop::FORWARD || m_sample->loop() == Sample::Loop::PING_PONG)
      {
        cairo_rectangle (cr, loop_start_x, 0, loop_end_x - loop_start_x, height);
        cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.25);
        cairo_fill (cr);
      }

    /* darken widget before and after clip region */
    cairo_rectangle (cr, 0, 0, clip_start_x, height);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.25);
    cairo_fill (cr);

    double effective_end_x = clip_end_x;
    if (m_sample->loop() == Sample::Loop::FORWARD || m_sample->loop() == Sample::Loop::PING_PONG)
      effective_end_x = loop_end_x;
    if (m_sample->loop() == Sample::Loop::SINGLE_FRAME)
      effective_end_x = loop_start_x;

    cairo_rectangle (cr, effective_end_x, 0, width - effective_end_x, height);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.25);
    cairo_fill (cr);

    du.set_color (Color (1.0, 0.3, 0.3));
    cairo_move_to (cr, 0, height/2);
    cairo_line_to (cr, width, height/2);
    cairo_stroke (cr);

    /* markers */
    for (int m = MARKER_LOOP_START; m <= MARKER_CLIP_END; m++)
      {
        MarkerType marker = static_cast<MarkerType> (m);
        double marker_x = m_sample->get_marker (marker) / length_ms * width;

        Rect  rect;
        Color color;
        if (m == MARKER_LOOP_START)
          {
            double c = 0;

            if (m_sample->loop() == Sample::Loop::NONE)
              continue;
            if (m_sample->loop() == Sample::Loop::SINGLE_FRAME) // center rect for single frame loop
              c = 5;

            rect = Rect (marker_x - c, 0, 10, 10);
            color = Color (0.7, 0.7, 1);

          }
        else if (m == MARKER_LOOP_END)
          {
            if (m_sample->loop() == Sample::Loop::NONE || m_sample->loop() == Sample::Loop::SINGLE_FRAME)
              continue;

            rect = Rect (marker_x - 10, 0, 10, 10);
            color = Color (0.7, 0.7, 1);
          }
        else if (m == MARKER_CLIP_START)
          {
            rect = Rect (marker_x, height - 10, 10, 10);
            color = Color (0.4, 0.4, 1);
          }
        else if (m == MARKER_CLIP_END)
          {
            if (m_sample->loop() != Sample::Loop::NONE)
              continue;

            rect = Rect (marker_x - 10, height - 10, 10, 10);
            color = Color (0.4, 0.4, 1);
          }
        marker_rect[marker] = rect;

        if (marker == selected_marker)
          color = color.lighter (175);
        du.set_color (color);

        cairo_rectangle (cr, rect.x(), rect.y(), rect.width(), rect.height());
        cairo_fill (cr);
        cairo_move_to (cr, marker_x, 0);
        cairo_line_to (cr, marker_x, height);
        cairo_stroke (cr);
      }
    for (auto p : m_play_pointers)
      {
        if (p > 0)
          {
            const double pos_x = play_pos_to_pixels (p);

            du.set_color (Color (1.0, 0.5, 0.0));
            cairo_move_to (cr, pos_x, 0);
            cairo_line_to (cr, pos_x, height);
            cairo_stroke (cr);
          }
      }
  }
  MarkerType
  find_marker_xy (double x, double y)
  {
    for (int m = MARKER_LOOP_START; m <= MARKER_CLIP_END; m++)
      {
        MarkerType marker = MarkerType (m);

        if (marker_rect[marker].contains (x, y))
          return marker;
      }
    return MARKER_NONE;
  }
  void
  get_order (MarkerType marker, std::vector<MarkerType>& left, std::vector<MarkerType>& right)
  {
    std::vector<MarkerType> left_to_right { MARKER_CLIP_START, MARKER_LOOP_START, MARKER_LOOP_END, MARKER_CLIP_END };

    std::vector<MarkerType>::iterator it = find (left_to_right.begin(), left_to_right.end(), marker);
    size_t pos = it - left_to_right.begin();

    left.clear();
    right.clear();

    for (size_t i = 0; i < left_to_right.size(); i++)
      {
        const MarkerType lr_marker = left_to_right[i];
        if (i < pos)
          left.push_back (lr_marker);
        if (i > pos)
          right.push_back (lr_marker);
      }
  }
  void
  motion (double x, double y) override
  {
    if (mouse_down)
      {
        if (selected_marker == MARKER_NONE)
          return;

        const double sample_len_ms = m_sample->wav_data.samples().size() / m_sample->wav_data.mix_freq() * 1000.0;
        const double x_ms = sm_bound<double> (0, x / width * sample_len_ms, sample_len_ms);

        m_sample->set_marker (selected_marker, x_ms);

        /* enforce ordering constraints */
        std::vector<MarkerType> left, right;
        get_order (selected_marker, left, right);

        for (auto l : left)
          if (m_sample->get_marker (l) > x_ms)
            m_sample->set_marker (l, x_ms);

        for (auto r : right)
          if (m_sample->get_marker (r) < x_ms)
            m_sample->set_marker (r, x_ms);

        update();
      }
    else
      {
        MarkerType old_marker = selected_marker;
        selected_marker = find_marker_xy (x, y);

        if (selected_marker != old_marker)
          update();
      }
  }
  void
  mouse_press (double x, double y) override
  {
    mouse_down = true;
  }
  void
  mouse_release (double x, double y) override
  {
    mouse_down = false;
    selected_marker = find_marker_xy (x, y);

    update();
  }
  void
  leave_event() override
  {
    selected_marker = MARKER_NONE;
    update();
  }
  void
  set_sample (Sample *sample)
  {
    m_sample = sample;
    update();
  }
  void
  set_vzoom (double factor)
  {
    vzoom = factor;
    update();
  }
  void
  update_markers()
  {
    update();
  }
  double
  play_pos_to_pixels (double pos_ms)
  {
    if (!m_sample)
      return -1;

    const double length_ms = m_sample->wav_data.samples().size() / m_sample->wav_data.mix_freq() * 1000;
    const double clip_start_ms = m_sample->get_marker (MARKER_CLIP_START);
    if (clip_start_ms > 0)
      pos_ms += clip_start_ms;

    const double pos_x = pos_ms / length_ms * width;
    return pos_x;
  }
  void
  set_play_pointers (const std::vector<float>& pointers)
  {
    if (pointers == m_play_pointers) /* save CPU power */
      return;

    std::vector<float> all_x;
    for (auto p : m_play_pointers)
      all_x.push_back (play_pos_to_pixels (p));

    m_play_pointers = pointers;

    for (auto p : m_play_pointers)
      all_x.push_back (play_pos_to_pixels (p));

    if (!all_x.empty())
      {
        double min_x = *std::min_element (all_x.begin(), all_x.end()) - 1;
        double max_x = *std::max_element (all_x.begin(), all_x.end()) + 1;
        double update_width = max_x - min_x;

        update (min_x, 0, update_width, height);
      }
  }
};

}
#endif
