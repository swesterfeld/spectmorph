// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_TIMEFREQVIEW_HH
#define SPECTMORPH_TIMEFREQVIEW_HH

#include <bse/bseloader.hh>
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
  bool        show_analysis;
  bool        m_show_frequency_grid;

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
  void set_position (int new_position);
  void set_display_params (double min_db, double boost);
  void set_show_analysis (bool new_show_analysis);
  void set_show_frequency_grid (bool new_show_frequency_grid);

  int     get_frames();
  double  get_progress();
  bool    show_frequency_grid();
  double  fundamental_freq();
  double  mix_freq();
  FFTResult get_spectrum();
  Audio  *audio();
  double  position_frac();

public slots:
  void on_result_available();
  void on_export();

signals:
  void spectrum_changed();
  void progress_changed();

#if 0 /* PORT */
  void load (const std::string& filename);
#endif
};

}

#endif /* SPECTMORPH_TIMEFREQVIEW_HH */
