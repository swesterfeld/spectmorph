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
#include "smaudio.hh"
#include "smfftthread.hh"

namespace SpectMorph {

class TimeFreqView : public Gtk::DrawingArea
{
protected:
  std::vector<FFTResult> results;
  PixelArray  image;
  Audio      *audio;
  Glib::RefPtr<Gdk::Pixbuf> zimage;
  double hzoom, vzoom;
  int position;
  bool show_analysis;

  void force_redraw();

  void on_timeout();

  FFTThread  fft_thread;

public:
  TimeFreqView (); //const std::string& filename);

  sigc::signal<void> signal_spectrum_changed;
  sigc::signal<void> signal_progress_changed;

  void load (const std::string& filename);
  void load (GslDataHandle *dhandle, const std::string& filename, Audio *audio, const AnalysisParams& analysis_params);
  bool on_expose_event (GdkEventExpose* ev);

  void set_zoom (double new_hzoom, double new_vzoom);
  void set_position (int new_position);
  void set_show_analysis (bool new_show_analysis);

  int  get_frames();
  FFTResult get_spectrum();
  double get_progress();
  static Glib::RefPtr<Gdk::Pixbuf> zoom_rect (PixelArray& image, int destx, int desty, int destw, int desth,
                                              double hzoom, double vzoom, int position);
};

}

#endif /* SPECTMORPH_TIMEFREQVIEW_HH */
