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

#include <gtkmm.h>

#include "smaudio.hh"
#include <bse/bseblockutils.hh>

namespace SpectMorph {

class SampleView : public Gtk::DrawingArea
{
  std::vector<float> signal;
  double hzoom;
  double vzoom;
  double attack_start;
  double attack_end;

  int old_width;
  void force_redraw();
public:
  SampleView();

  sigc::signal<void, int, int> signal_resized;

  void load (GslDataHandle *dhandle, SpectMorph::Audio *audio);

  bool on_expose_event (GdkEventExpose *ev);
  void set_zoom (double hzoom, double vzoom);

  template<class DrawOps> static void
  draw_signal (std::vector<float>& signal, DrawOps ops, GdkEventExpose *ev, int height, double vz, double hz)
  {
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
#if 0
    int last_x = 0;
    double last_value = 0, min_value = 0, max_value = 0;
    for (size_t i = 0; i < signal.size(); i++)
      {
        double value = signal[i];
        int x = hz * i;
        if (x == last_x)
          {
            if (value < min_value)
              min_value = value;
            if (value > max_value)
              max_value = value;
          }
        else
          {
            if (last_x >= ev->area.x && x < ev->area.x + ev->area.width)
              {
                if (min_value != max_value)
                  {
                    ops->move_to (last_x, (height / 2) + min_value * vz);
                    ops->line_to (last_x, (height / 2) + max_value * vz);
                  }
                ops->move_to (last_x, (height / 2) + last_value * vz);
                ops->line_to (x, (height / 2) + value * vz);
              }
            min_value = value;
            max_value = value;
          }
        last_value = value;
        last_x = x;
      }
#endif
  }
};

}

#endif
