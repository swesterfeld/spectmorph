// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SPECTRUMVIEW_HH
#define SPECTMORPH_SPECTRUMVIEW_HH

#include "smtimefreqview.hh"

namespace SpectMorph {

class Navigator;
class SpectrumView : public QWidget
{
  Q_OBJECT

  Navigator    *navigator;
  double        hzoom;
  double        vzoom;
  TimeFreqView *time_freq_view_ptr;
  FFTResult     spectrum;
  AudioBlock    audio_block;
public:
  SpectrumView (Navigator *navigator);

  void update_size();
  void set_zoom (double hzoom, double vzoom);
  void set_spectrum_model (TimeFreqView *tfview);
  void paintEvent (QPaintEvent *event);

public slots:
  void on_display_params_changed();
  void on_spectrum_changed();
};

}

#endif
