// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SAMPLE_WIN_VIEW_HH

#include "smsampleview.hh"
#include "smzoomcontroller.hh"
#include <QWidget>
#include <QScrollArea>
#include <QPushButton>
#include <QComboBox>

namespace SpectMorph
{

class Navigator;

class SampleWinView : public QWidget
{
  Q_OBJECT

  ZoomController *zoom_controller;
  SampleView     *m_sample_view;
  QScrollArea    *scroll_area;

  QLabel         *time_label;
  QPushButton    *edit_loop_start;
  QPushButton    *edit_loop_end;
  QComboBox      *loop_type_combo;

  Navigator      *navigator;
public:
  SampleWinView (Navigator *navigator);

  void load (const WavData *wav_data, SpectMorph::Audio *audio);
  SampleView *sample_view();

public slots:
  void on_zoom_changed();
  void on_mouse_time_changed (int new_time);
  void on_edit_marker_changed();
  void on_loop_type_changed();

signals:
  void audio_edit();
};

}

#endif

