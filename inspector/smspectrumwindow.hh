// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SPECTRUMWINDOW_HH
#define SPECTMORPH_SPECTRUMWINDOW_HH

#include "smspectrumview.hh"
#include "smtimefreqview.hh"
#include "smzoomcontroller.hh"

#include <QScrollArea>

namespace SpectMorph {

class Navigator;
class SpectrumWindow : public QWidget
{
  Q_OBJECT

  QScrollArea        *scroll_area;
  ZoomController     *zoom_controller;
  SpectrumView       *spectrum_view;
public:
  SpectrumWindow (Navigator *navigator);

  void set_spectrum_model (TimeFreqView *time_freq_view);
public slots:
  void on_zoom_changed();
};

}

#endif
