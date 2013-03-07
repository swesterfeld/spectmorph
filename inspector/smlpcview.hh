// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LPC_VIEW_HH
#define SPECTMORPH_LPC_VIEW_HH

#include "smtimefreqview.hh"

namespace SpectMorph {

class LPCView : public QWidget
{
  Q_OBJECT

  TimeFreqView *time_freq_view_ptr;
  AudioBlock    audio_block;
  double        hzoom;
  double        vzoom;

  void update_size();

public:
  LPCView();

  void paintEvent (QPaintEvent *event);

  void set_lpc_model (TimeFreqView *tfview);
  void set_zoom (double hzoom, double vzoom);

public slots:
  void on_lpc_changed();
};

}

#endif
