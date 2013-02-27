/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

  void               update_size();

public:
  SampleView();
  void load (GslDataHandle *dhandle, SpectMorph::Audio *audio, Markers *markers = 0);
  void set_zoom (double hzoom, double vzoom);
  void paintEvent (QPaintEvent *event);

  void set_edit_marker_type (EditMarkerType marker_type);
  EditMarkerType edit_marker_type();

  template<class DrawOps> static void
  draw_signal (std::vector<float>& signal, DrawOps ops, /* GdkEventExpose *ev, */ int height, double vz, double hz)
  {
#if 0
    int x = ev->area.x;
    int last_i0 = -1;
    int last_x = 0;
    double last_value = 0;
    while (x < ev->area.x + ev->area.width)
      {
        int i0 = x / hz;
        int i1 = (x + 1) / hz + 1;

        if (last_i0 != i0)
          {
            if (i0 < int (signal.size()) && i0 >= 0 && i1 < int (signal.size() + 1) && i1 > 0)
              {
                ops->move_to (last_x, (height / 2) + last_value * vz);
                ops->line_to (x, (height / 2) + signal[i0] * vz);

                float min_value, max_value;
                Bse::Block::range (i1 - i0, &signal[i0], min_value, max_value);

                ops->move_to (x, (height / 2) + min_value * vz);
                ops->line_to (x, (height / 2) + max_value * vz);

                last_x = x;
                last_value = signal[i1 - 1];
              }
            last_i0 = i0;
          }
        x++;
      }
#endif
  }
#if 0
private:
  double hzoom;
  double vzoom;
  bool   button_1_pressed;

  int old_width;
  void force_redraw();
  void move_marker (int x);
public:
  SampleView();

  //sigc::signal<void, int, int> signal_resized;
  //sigc::signal<void>           signal_audio_edit;
  //sigc::signal<void, int>      signal_mouse_time_changed;

  void load (GslDataHandle *dhandle, SpectMorph::Audio *audio, Markers *markers = 0);

  bool on_expose_event (GdkEventExpose *ev);
  bool on_button_press_event (GdkEventButton *event);
  bool on_motion_notify_event (GdkEventMotion *event);
  bool on_button_release_event (GdkEventButton *event);

  void set_zoom (double hzoom, double vzoom);

  void set_edit_marker_type (EditMarkerType marker_type);
  EditMarkerType edit_marker_type();

#endif
};

}

#endif
