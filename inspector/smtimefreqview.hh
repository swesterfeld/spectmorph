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

#include <bse/bseloader.h>
#include "smpixelarray.hh"
#include "smaudio.hh"
#include "smfftthread.hh"

#include <QWidget>

namespace SpectMorph {

class TimeFreqView : public QWidget
{
  Q_OBJECT
protected:
  PixelArray  image;
  Audio      *m_audio;
  double      hzoom;
  double      vzoom;
  double      display_min_db;
  double      display_boost;
  int         position;

  FFTThread  fft_thread;

  void scale_zoom (double *scaled_hzoom, double *scaled_vzoom);

public:
  TimeFreqView();

  void load (GslDataHandle *dhandle, const std::string& filename, Audio *audio, const AnalysisParams& analysis_params);
  void update_size();
  void paintEvent (QPaintEvent *event);

  static QImage zoom_rect (PixelArray& image, int destx, int desty, int destw, int desth,
                           double hzoom, double vzoom, int position,
                           double display_min_db, double display_boost);
  void set_zoom (double new_hzoom, double new_vzoom);

public slots:
  void on_result_available();

signals:
  void spectrum_changed();
  void progress_changed();

#if 0 /* PORT */
  int old_height;
  int old_width;
  bool show_analysis;
  bool m_show_frequency_grid;

  void force_redraw();



public:
  TimeFreqView (); //const std::string& filename);

  sigc::signal<void, int, int, int, int> signal_resized;

  void load (const std::string& filename);
  void load (GslDataHandle *dhandle, const std::string& filename, Audio *audio, const AnalysisParams& analysis_params);
  bool on_expose_event (GdkEventExpose* ev);

  void set_position (int new_position);
  void set_show_analysis (bool new_show_analysis);
  void set_show_frequency_grid (bool new_show_frequency_grid);
  void set_display_params (double min_db, double boost);

  int  get_frames();
  FFTResult get_spectrum();
  double get_progress();
  bool show_frequency_grid();
  double fundamental_freq();
  double mix_freq();
  Audio *audio();
  double position_frac();
#endif
};

}

#endif /* SPECTMORPH_TIMEFREQVIEW_HH */
