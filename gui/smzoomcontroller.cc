// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smzoomcontroller.hh"
#include <math.h>
#include <stdio.h>

#include <QSlider>
#include <QVBoxLayout>

using namespace SpectMorph;

ZoomController::ZoomController (double hzoom_max, double vzoom_max)
{
  init();
  hzoom_slider->setRange (-1000, (log10 (hzoom_max) - 2) * 1000);
  vzoom_slider->setRange (-1000, (log10 (vzoom_max) - 2) * 1000);
}

ZoomController::ZoomController (double hzoom_min, double hzoom_max, double vzoom_min, double vzoom_max)
{
  init();
  hzoom_slider->setRange ((log10 (hzoom_min) - 2) * 1000, (log10 (hzoom_max) - 2) * 1000);
  vzoom_slider->setRange ((log10 (vzoom_min) - 2) * 1000, (log10 (vzoom_max) - 2) * 1000);
}

void
ZoomController::init()
{
  vscrollbar = NULL;
  hscrollbar = NULL;

  hzoom_slider = new QSlider (Qt::Horizontal);
  hzoom_label  = new QLabel();
  vzoom_slider = new QSlider (Qt::Horizontal);
  vzoom_label  = new QLabel();

  QGridLayout *grid_layout = new QGridLayout();

  grid_layout->addWidget (new QLabel ("HZoom"), 0, 0);
  grid_layout->addWidget (hzoom_slider, 0, 1);
  grid_layout->addWidget (hzoom_label, 0, 2);
  grid_layout->addWidget (new QLabel ("VZoom"), 1, 0);
  grid_layout->addWidget (vzoom_slider, 1, 1);
  grid_layout->addWidget (vzoom_label, 1, 2);
  setLayout (grid_layout);

  connect (hzoom_slider, SIGNAL (valueChanged (int)), this, SLOT (on_hzoom_changed()));
  connect (vzoom_slider, SIGNAL (valueChanged (int)), this, SLOT (on_vzoom_changed()));

  old_hzoom = 1;
  old_vzoom = 1;

  on_hzoom_changed();
  on_vzoom_changed();
}

double
ZoomController::get_hzoom()
{
  return pow (10, hzoom_slider->value() / 1000.0);
}

double
ZoomController::get_vzoom()
{
  return pow (10, vzoom_slider->value() / 1000.0);
}

void
ZoomController::on_hzoom_changed()
{
  double hzoom = get_hzoom();
  char buffer[1024];
  sprintf (buffer, "%3.2f%%", 100.0 * hzoom);
  hzoom_label->setText (buffer);

  if (hscrollbar)
    {
      const double hfactor = hzoom / old_hzoom;
      hscrollbar->setValue (hfactor * hscrollbar->value() + ((hfactor - 1) * hscrollbar->pageStep() / 2));
    }
  old_hzoom = hzoom;
  emit zoom_changed();
}

void
ZoomController::on_vzoom_changed()
{
  double vzoom = get_vzoom();
  char buffer[1024];
  sprintf (buffer, "%3.2f%%", 100.0 * vzoom);
  vzoom_label->setText (buffer);

  if (vscrollbar)
    {
      const double vfactor = vzoom / old_vzoom;
      vscrollbar->setValue (vfactor * vscrollbar->value() + ((vfactor - 1) * vscrollbar->pageStep() / 2));
    }
  old_vzoom = vzoom;
  emit zoom_changed();
  emit zoom_changed();
}

void
ZoomController::set_vscrollbar (QScrollBar *scrollbar)
{
  vscrollbar = scrollbar;
}

void
ZoomController::set_hscrollbar (QScrollBar *scrollbar)
{
  hscrollbar = scrollbar;
}
