// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LPC_WINDOW_HH
#define SPECTMORPH_LPC_WINDOW_HH

#include "smlpcview.hh"
#include "smtimefreqview.hh"
#include "smzoomcontroller.hh"

#include <QScrollArea>

namespace SpectMorph {

class LPCWindow : public QWidget
{
  Q_OBJECT

  QScrollArea      *scroll_area;
  LPCView          *lpc_view;
  ZoomController   *zoom_controller;

public:
  LPCWindow();

  void set_lpc_model (TimeFreqView *time_freq_view);

public slots:
  void on_zoom_changed();
};

}

#endif
