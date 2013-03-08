// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_ZOOMCONTROLLER_HH
#define SPECTMORPH_ZOOMCONTROLLER_HH

#include <QWidget>
#include <QSlider>
#include <QScrollBar>
#include <QLabel>

namespace SpectMorph {

class ZoomController : public QObject
{
  Q_OBJECT

  double   old_hzoom;
  double   old_vzoom;

  QLabel  *hzoom_text;
  QLabel  *vzoom_text;

  QSlider *hzoom_slider;
  QLabel  *hzoom_label;
  QSlider *vzoom_slider;
  QLabel  *vzoom_label;

  QScrollBar  *vscrollbar;
  QScrollBar  *hscrollbar;

  void init();
public:
  ZoomController (QObject *parent, double hzoom_max = 1000.0, double vzoom_max = 1000.0);
  ZoomController (QObject *parent, double hzoom_min, double hzoom_max, double vzoom_min, double vzoom_max);

  QWidget *hwidget (int i);
  QWidget *vwidget (int i);

  double get_hzoom();
  double get_vzoom();

  void set_vscrollbar (QScrollBar *scrollbar);
  void set_hscrollbar (QScrollBar *scrollbar);

public slots:
  void on_hzoom_changed();
  void on_vzoom_changed();

signals:
  void zoom_changed();

};

}

#endif
