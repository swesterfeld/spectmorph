// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SAMPLE_WIN_VIEW_HH

#include "smsampleview.hh"
#include "smzoomcontroller.hh"
#include <QWidget>
#include <QScrollArea>

namespace SpectMorph
{

class SampleWinView : public QWidget
{
  Q_OBJECT

  ZoomController *zoom_controller;
  SampleView     *sample_view;
  QScrollArea    *scroll_area;
public:
  SampleWinView();

  void load (GslDataHandle *dhandle, SpectMorph::Audio *audio);

public slots:
  void on_zoom_changed();
};

}

#endif

