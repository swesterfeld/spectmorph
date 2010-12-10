/*
 * Copyright (C) 2010 Stefan Westerfeld
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

#ifndef SPECTMORPH_TIMEFREQVIEW_HH
#define SPECTMORPH_TIMEFREQVIEW_HH

#include <gtkmm.h>
#include <bse/bseloader.h>
#include "smpixelarray.hh"

namespace SpectMorph {

struct FFTResult
{
  std::vector<float> mags;
};

struct AnalysisParams
{
  double frame_size_ms;
  double frame_step_ms;
};

class TimeFreqView : public Gtk::DrawingArea
{
protected:
  std::vector<FFTResult> results;
  PixelArray  image;
  Glib::RefPtr<Gdk::Pixbuf> zimage;
  double hzoom, vzoom;
  int position;

  void force_redraw();

public:
  TimeFreqView (); //const std::string& filename);

  sigc::signal<void> signal_spectrum_changed;

  void load (const std::string& filename);
  void load (GslDataHandle *dhandle, const std::string& filename);
  bool on_expose_event (GdkEventExpose* ev);

  void set_zoom (double new_hzoom, double new_vzoom);
  void set_position (int new_position);

  int  get_frames();
  FFTResult get_spectrum();
  static Glib::RefPtr<Gdk::Pixbuf> zoom_rect (PixelArray& image, int destx, int desty, int destw, int desth,
                                              double hzoom, double vzoom, int position);
};

}

#endif /* SPECTMORPH_TIMEFREQVIEW_HH */
