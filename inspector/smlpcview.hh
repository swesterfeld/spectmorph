/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
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

#ifndef SPECTMORPH_LPC_VIEW_HH
#define SPECTMORPH_LPC_VIEW_HH

#include <gtkmm.h>
#include "smtimefreqview.hh"

namespace SpectMorph {

class LPCView : public Gtk::DrawingArea
{
  TimeFreqView *time_freq_view_ptr;
  AudioBlock    audio_block;
  double        hzoom;
  double        vzoom;

  void force_redraw();

public:
  LPCView();

  bool on_expose_event (GdkEventExpose* ev);

  void set_lpc_model (TimeFreqView& tfview);
  void on_lpc_changed();

  void set_zoom (double hzoom, double vzoom);
};

}

#endif
