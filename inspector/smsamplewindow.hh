// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SAMPLE_WINDOW_HH
#define SPECTMORPH_SAMPLE_WINDOW_HH

#include "smsampleview.hh"
#include "smzoomcontroller.hh"
#include "smwavset.hh"

#include <QMainWindow>

namespace SpectMorph {

class Navigator;
class SampleWinView;
class SampleWindow : public QMainWindow
{
  Q_OBJECT

private:
  Navigator          *navigator;
  SampleWinView      *sample_win_view;

public:
  SampleWindow (Navigator *navigator);

  void        load (GslDataHandle *dhandle, Audio *audio);
  SampleView *sample_view();

public slots:
  void on_dhandle_changed();
  void on_next_sample();

signals:
  void next_sample();
};

}

#endif
