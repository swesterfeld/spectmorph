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

namespace SpectMorph {

class SampleView : public Gtk::DrawingArea
{
  std::vector<float> signal;
  double hzoom;
  double vzoom;
  double attack_start;
  double attack_end;

  int old_width, old_height;
  void force_redraw();
public:
  SampleView();

  sigc::signal<void, int, int, int, int> signal_resized;

  void load (GslDataHandle *dhandle, SpectMorph::Audio *audio);

  bool on_expose_event (GdkEventExpose *ev);
  void set_zoom (double hzoom, double vzoom);
};

}

#endif
