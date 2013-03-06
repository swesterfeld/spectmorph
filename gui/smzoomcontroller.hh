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

#ifndef SPECTMORPH_ZOOMCONTROLLER_HH
#define SPECTMORPH_ZOOMCONTROLLER_HH

#include <QWidget>
#include <QSlider>
#include <QScrollBar>
#include <QLabel>

namespace SpectMorph {

class ZoomController : public QWidget
{
  Q_OBJECT

  double   old_hzoom;
  double   old_vzoom;

  QSlider *hzoom_slider;
  QLabel  *hzoom_label;
  QSlider *vzoom_slider;
  QLabel  *vzoom_label;

  QScrollBar  *vscrollbar;
  QScrollBar  *hscrollbar;

  void init();
public:
  ZoomController (double hzoom_max = 1000.0, double vzoom_max = 1000.0);
  ZoomController (double hzoom_min, double hzoom_max, double vzoom_min, double vzoom_max);

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
