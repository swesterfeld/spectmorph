// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SAMPLE_VIEW_HH
#define SPECTMORPH_SAMPLE_VIEW_HH

#include "smaudio.hh"
#include <bse/bseblockutils.hh>

#include <QWidget>

namespace SpectMorph {

class SampleView : public QWidget
{
  Q_OBJECT

public:
  enum EditMarkerType {
    MARKER_NONE,
    MARKER_START,
    MARKER_LOOP_START,
    MARKER_LOOP_END,
    MARKER_CLIP_START,
    MARKER_CLIP_END
  };
  class Markers {
  public:
    virtual size_t          count() = 0;
    virtual EditMarkerType  type (size_t marker) = 0;
    virtual float           position (size_t marker) = 0;
    virtual bool            valid (size_t marker) = 0;
    virtual void            set_position (size_t marker, float new_position) = 0;
    virtual void            clear (size_t marker) = 0;
  };

private:
  std::vector<float> signal;
  Audio             *audio;
  Markers           *markers;
  double             attack_start;
  double             attack_end;
  double             hzoom;
  double             vzoom;
  EditMarkerType     m_edit_marker_type;
  bool               button_1_pressed;

  void               update_size();
  void               mousePressEvent (QMouseEvent *event);
  void               move_marker (int x);
  void               mouseMoveEvent (QMouseEvent *event);
  void               mouseReleaseEvent (QMouseEvent *event);

public:
  SampleView();
  void load (GslDataHandle *dhandle, SpectMorph::Audio *audio, Markers *markers = 0);
  void set_zoom (double hzoom, double vzoom);
  void paintEvent (QPaintEvent *event);

  void set_edit_marker_type (EditMarkerType marker_type);
  EditMarkerType edit_marker_type();

  template<class Painter> static void
  draw_signal (std::vector<float>& signal, Painter& painter, const QRect& rect, int height, double vz, double hz)
  {
    int last_i0 = -1;
    int last_x = 0;
    double last_value = 0;

    for (int x = rect.x(); x < rect.x() + rect.width(); x++)
      {
        int i0 = x / hz;
        int i1 = (x + 1) / hz + 1;

        if (last_i0 != i0)
          {
            if (i0 < int (signal.size()) && i0 >= 0 && i1 < int (signal.size() + 1) && i1 > 0)
              {
                painter.drawLine (last_x, (height / 2) + last_value * vz, x, (height / 2) + signal[i0] * vz);

                float min_value, max_value;
                Bse::Block::range (i1 - i0, &signal[i0], min_value, max_value);

                painter.drawLine (x, (height / 2) + min_value * vz, x, (height / 2) + max_value * vz);

                last_x = x;
                last_value = signal[i1 - 1];
              }
            last_i0 = i0;
          }
      }
  }
signals:
  void audio_edit();
  void mouse_time_changed (int pos);
};

}

#endif
